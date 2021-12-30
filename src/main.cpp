#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <LITTLEFS.h>

#include "main.h"
#include "timeStuff.h"
#include "NTP.h"
#include "settings.h"
#include "Nextion/Nextion.h"
#include "NexClock.h"
#include "inetradio.h"

QueueHandle_t xQueue;

// Interrupts.
hw_timer_t *                timerLoop             = NULL;
volatile SemaphoreHandle_t  timerLoopSemaphore;

bool wiFiDisconnected = true;


int oldSeconds = -1;
int oldMinutes = -1;
int oldDay = -1;


//
// onTimer
// Called by interrupt service.
// Entry   : nothing
// Returns : nothing
// Notes   :
//
//  1) Via xSemaphoreGiveFromISR(), enable the next loop() pass.
//
void IRAM_ATTR onTimer()
{
  // Give semaphore.
  xSemaphoreGiveFromISR(timerLoopSemaphore, NULL);
}


// Convert the WiFi (error) response to a string we can understand
const char *wl_status_to_string(wl_status_t status)
{
	switch (status)
	{
	case WL_NO_SHIELD:
		return "WL_NO_SHIELD";
	case WL_IDLE_STATUS:
		return "WL_IDLE_STATUS";
	case WL_NO_SSID_AVAIL:
		return "WL_NO_SSID_AVAIL";
	case WL_SCAN_COMPLETED:
		return "WL_SCAN_COMPLETED";
	case WL_CONNECTED:
		return "WL_CONNECTED";
	case WL_CONNECT_FAILED:
		return "WL_CONNECT_FAILED";
	case WL_CONNECTION_LOST:
		return "WL_CONNECTION_LOST";
	case WL_DISCONNECTED:
		return "WL_DISCONNECTED";
	default:
		return "UNKNOWN";
	}
}


// Connect to WiFi
bool connectToWiFi()
{
  log_w("Start WiFi.");
	log_w("Connecting to SSID: %s\n", ClockSettings.szSSID);

	// Ensure we disconnect WiFi first to stop connection problems
	if (WiFi.status() == WL_CONNECTED)
	{
		log_w("Disconnecting from WiFi");
		WiFi.disconnect(false, true);
	}

	// We want to be a client not a server
	WiFi.mode(WIFI_STA);

	// Don't store the SSID and password
	WiFi.persistent(false);

	// Don't allow WiFi sleep
	WiFi.setSleep(false);

	WiFi.setAutoReconnect(false);

	// Lock down the WiFi stuff - not mormally necessary unless you need a static IP in AP mode
	// IPAddress local_IP(192, 168, 1, 102);
	// IPAddress gateway(192, 168, 1, 254);
	// IPAddress subnet(255, 255, 255, 0);
	// IPAddress DNS1(8, 8, 8, 8);
	// IPAddress DNS2(8, 8, 4, 4);
	// WiFi.config(local_IP, gateway, subnet, DNS1, DNS2);

	//Connect to the required WiFi
	WiFi.begin(ClockSettings.szSSID, ClockSettings.szPWD);

	// Give things a chance to settle, avoid problems
	delay(2000);

	log_w("Waiting for WiFi connection...");
	uint8_t wifiStatus = WiFi.waitForConnectResult();

	// Successful connection?
	if ((wl_status_t)wifiStatus != WL_CONNECTED)
	{
		log_w("WiFi Status: %s, exiting", wl_status_to_string((wl_status_t)wifiStatus));
		return false;
	}

	WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);

	Serial.printf("WiFi connected with (local) IP address of: %s\n", WiFi.localIP().toString().c_str());
	wiFiDisconnected = false;

  return true;
}

void displayTime()
{
  char msg[128];

  snprintf(msg, sizeof(msg), "%02d:%02d %cm", 
    ClockSettings.b24Hour ? (tmTime.tm_hour + tmTime.tm_isdst) % 24 : ((tmTime.tm_hour + tmTime.tm_isdst) % 12) ? ((tmTime.tm_hour + tmTime.tm_isdst) % 12) : 12, 
    tmTime.tm_min,
    ((tmTime.tm_hour + tmTime.tm_isdst) % 24) >= 12 ? 'p' : 'a');

  pRadio_Time.setText(msg);

  if (!ClockSettings.b24Hour)
    if (msg[0] == '0') msg[0] = ' ';
  pClock_t3.setText(String(msg[0]).c_str());
  pClock_t4.setText(String(msg[1]).c_str());
  pClock_t5.setText(String(msg[3]).c_str());
  pClock_t6.setText(String(msg[4]).c_str());
  

  if (ClockSettings.b24Hour)
  {
    pClock_t1.setText(" ");
    pClock_t2.setText(" ");
  }
  else
  {
    if (((tmTime.tm_hour + tmTime.tm_isdst) % 24) >= 12)
    {
      pClock_t1.setText(" ");
      pClock_t2.setText("PM");
    }    
    else
    {
      pClock_t1.setText("AM");
      pClock_t2.setText("  ");
    }
  }
}


void displayDate()
{
  char msg[128];

  snprintf(msg, sizeof(msg), "%s %s %d, %4d", chDaysOfWeek[tmTime.tm_wday], chMonth[tmTime.tm_mon], tmTime.tm_mday, tmTime.tm_year);
  pClock_t7.setText(msg);
}

TaskHandle_t NexClockLoopTaskHandle;

void NexClockLoopTask(void *parameter)
{
  static uint8_t __buffer[10];
  
  uint16_t i;
  uint8_t c;  
  
  while (true)
  {
    while (nexSerial.available() > 0)
    {   
      delay(10);
      c = nexSerial.read();

      if (0x65 == c)
      {
        if (nexSerial.available() >= 6)
        {
          __buffer[0] = c;  
          for (i = 1; i < 7; i++)
          {
            __buffer[i] = nexSerial.read();
          }
          __buffer[i] = 0x00;

          if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
          {
            NexObject::iterate(nex_listen_list, __buffer[1], __buffer[2], (int32_t)__buffer[3]);
          }
        }
      }
    }

    vTaskDelay(1);
  }
}


void setup() 
{
  char msg[256];

  Serial.begin(115200);
  delay(500);
  log_w("\n\n\n*** Starting *************************************************************************\n\n");

  ////////////////////////////////////////////////////////
  // Start Nextion display

  if (!NexClockInit(25, 27))
  {
    log_w("Nextion display initialization failed");
  }

  delay(500);

  pageStartup.show();
  
  NexClockAttachCallbacks();

  ////////////////////////////////////////////////////////
  // Start Filesystem and read settings
  SPI.begin();

  if (!StartLITTLEFS())
  {
    log_w("Unable to starrt LITTLEFS");
    while (true) delay(10000);
  }

  //listDir("/");
  //LITTLEFS.remove("/conf");
  //listDir("/");


  if (!readSettings())
  {
    log_w("Unable to read settings");
    while (true) delay(10000);    
  }

  //dumpSettings();


  ApplySettingToPage();   // Now that the display is setup, we can apply the settings to the Setup page
  
  ////////////////////////////////////////////////////////
  // Start WiFi and connect to SSID
  pStartup_t0.setText("WiFi Connecting to Oxford");
  while (!connectToWiFi())
    delay(1000);

  log_w("setup: IP Address %s", WiFi.localIP().toString().c_str());

  snprintf(msg, sizeof(msg), "Connected to %s", "Oxford - HASP");
  pStartup_t0.setText(msg);
  snprintf(msg, sizeof(msg), "%s", WiFi.localIP().toString().c_str());
  pStartup_t1.setText(msg);

  ////////////////////////////////////////////////////////
  // Start Over-The-Air system
  ArduinoOTA.onStart([]() 
  {
    char msg[128];
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "firmware";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    sendCommand("page Upload");
    snprintf(msg, sizeof(msg), "Start updating %s", type.c_str());
    pUpload_t0.setText(msg);
    pUpload_t1.setText("");

    vTaskSuspend(playMusicTaskHandle);
    vTaskSuspend(NexClockLoopTaskHandle);

    log_w("ArduinoOTA: %s", msg);
  });
  ArduinoOTA.onEnd([]() 
  {
    log_w("\nArduinoOTA: End");
    pUpload_t1.setText("Update finished");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    Serial.printf("ArduinoOTA: Progress: %u%%\r", (progress / (total / 100)));
    setUploadProgressBar("j0", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) 
  {
    String smsg;

    smsg = "ArduinoOTA: Error[" + error;
    smsg +=  "]: ";
         if (error == OTA_AUTH_ERROR)    smsg +=  "Auth Failed";
    else if (error == OTA_BEGIN_ERROR)   smsg +=  "Begin Failed";
    else if (error == OTA_CONNECT_ERROR) smsg +=  "Connect Failed";
    else if (error == OTA_RECEIVE_ERROR) smsg +=  "Receive Failed";
    else if (error == OTA_END_ERROR)     smsg +=  "End Failed";

    log_w("%s", smsg.c_str());
    pUpload_t1.setText(smsg.c_str());
    delay(5000);
    ESP.restart();
  });

  ArduinoOTA.begin();

  ////////////////////////////////////////////////////////
  // Start Network Time Protocol system
  if (!StartNTP())
  {
    log_w("NTP failed to start");
    while (1)
      yield();
  }

  ////////////////////////////////////////////////////////
  // Interrupt service.
  // Create onTimer() and loop() semaphore.
  timerLoopSemaphore = xSemaphoreCreateBinary();

  // Use timer 0 (4 available from 0 through 3), with a prescale of 80.
  timerLoop = timerBegin(0, 80, true);
    
  // Attach onTimer() to timer.
  timerAttachInterrupt(timerLoop, &onTimer, true);
    
  // Set alarm to call onTimer function every second (value in microseconds)
  // with repeat (third parameter).
  timerAlarmWrite(timerLoop, 1000000, true);
    
  // Start the alarm.
  timerAlarmEnable(timerLoop);


  ////////////////////////////////////////////////////////
  // Get the current time
  pStartup_t2.setText("Getting current time");
  sendCommand("vis z0,1");
  int step = 18;
  int pos = 0;
  while (!bTimeIsValid)
  {
    UpdateTime(&tmTime);
    delay(100);
  
    pos += step;
    pos %= 360;
    pStartup_z0.setValue(pos);
  }
  sendCommand("vis z0,0");

  pStartup_t2.setFont(0);
  snprintf(msg, sizeof(msg), "%s, %s %d%s, %4d, %02d:%02d:%02d%s",
    chDaysOfWeek[tmTime.tm_wday],
    chMonth[tmTime.tm_mon],
    tmTime.tm_mday,
    (tmTime.tm_mday == 1) ? "st" : (tmTime.tm_mday == 2) ? "nd" : (tmTime.tm_mday == 3) ? "rd" : "th",
    tmTime.tm_year,
    ((tmTime.tm_hour + tmTime.tm_isdst) % 12) ? ((tmTime.tm_hour + tmTime.tm_isdst) % 12) : 12,
    tmTime.tm_min,
    tmTime.tm_sec,
    (((tmTime.tm_hour + tmTime.tm_isdst) % 24) >= 12) ? "pm" : "am");
  pStartup_t2.setText(msg);

  displayTime();
  displayDate();

  delay(50);

  log_w("loop(): date and time  : %s, %s %d%s, %4d, %02d:%02d:%02d%s, dst is %s.",
    chDaysOfWeek[tmTime.tm_wday],
    chMonth[tmTime.tm_mon],
    tmTime.tm_mday,
    (tmTime.tm_mday == 1) ? "st" : (tmTime.tm_mday == 2) ? "nd" : (tmTime.tm_mday == 3) ? "rd" : "th",
    tmTime.tm_year,
    ((tmTime.tm_hour + tmTime.tm_isdst) % 12) ? ((tmTime.tm_hour + tmTime.tm_isdst) % 12) : 12,
    tmTime.tm_min,
    tmTime.tm_sec,
    (((tmTime.tm_hour + tmTime.tm_isdst) % 24) >= 12) ? "pm" : "am",
    (tmTime.tm_isdst) ? "on" : "off");



  ////////////////////////////////////////////////////////
  // Start MP3 player
  pStartup_t3.setText("Starting radio");
  mp3Start();

 	xTaskCreatePinnedToCore(
		NexClockLoopTask,		  /* Function to implement the task */
		"NexClockLoopTask",		  /* Name of the task */
		3000,				  /* Stack size in words */
		NULL,				  /* Task input parameter */
		1,					  /* Priority of the task - must be higher than 0 (idle)*/
		&NexClockLoopTaskHandle, /* Task handle. */
		1);					  /* Core where the task should run */

  ////////////////////////////////////////////////////////
  // Switch to Clock display
  pClock_SongTitleArtist.setText("");
  pageClock.show();

  log_w("\n\n******** setup finished\n\n");
}


void loop() 
{
  ArduinoOTA.handle();

  handleIncomingRadioData();

  // Wait for alarm tick (set to 1hz. in setup()).  Keep looping until we get the signal.
  if(xSemaphoreTake(timerLoopSemaphore, ( TickType_t ) 1) == pdTRUE)
  {
    // Update time.
    UpdateTime(&tmTime);
  }

  handleIncomingRadioData();


  if (oldSeconds != tmTime.tm_sec)
  {
    if (ClockSettings.bFlashColon)
    {
      if (tmTime.tm_sec % 2)
      {
        pClock_t0.setText(":");
      }
      else
      {
        pClock_t0.setText(" ");
      }
    }
    else
      pClock_t0.setText(":");

    oldSeconds = tmTime.tm_sec;
  }

  handleIncomingRadioData();

  if (oldMinutes != tmTime.tm_min)
  {
    displayTime();
    writeSettings();
    oldMinutes = tmTime.tm_min;
  }

  handleIncomingRadioData();

  if (oldDay != tmTime.tm_mday)
  {
    oldDay = tmTime.tm_mday;
    displayDate();
  }

  handleIncomingRadioData();
}


