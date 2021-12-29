#include <Arduino.h>

#include "main.h"

#include "settings.h"
#include "inetradio.h"
#include "NexClock.h"


NexPage pageStartup = NexPage(0, 0, "Startup");
NexPage pageClock   = NexPage(1, 0, "Clock");
NexPage pageSetup   = NexPage(2, 0, "Setup");
NexPage pageAlarm   = NexPage(3, 0, "Alarm");
NexPage pageUpload  = NexPage(4, 0, "Upload");
NexPage pageRadio   = NexPage(5, 0, "Radio");

// Page 0 - Startup
NexText pStartup_t0 = NexText(&pageStartup, 0, "t0");
NexText pStartup_t1 = NexText(&pageStartup, 1, "t1");
NexText pStartup_t2 = NexText(&pageStartup, 2, "t2");
NexText pStartup_t3 = NexText(&pageStartup, 5, "t3");

NexGauge pStartup_z0 = NexGauge(0, 0, "z0");

NexVariable pStartup_radioState = NexVariable(0, 6, "radioState");


// Page 1 - Clock
NexText pClock_t0 = NexText(&pageClock, 7, "t0");
NexText pClock_t1 = NexText(&pageClock, 1, "t1");
NexText pClock_t2 = NexText(&pageClock, 2, "t2");
NexText pClock_t3 = NexText(&pageClock, 3, "t3");
NexText pClock_t4 = NexText(&pageClock, 4, "t4");
NexText pClock_t5 = NexText(&pageClock, 5, "t5");
NexText pClock_t6 = NexText(&pageClock, 6, "t6");
NexText pClock_t7 = NexText(&pageClock, 8, "t7");

NexButton pClock_b0 = NexButton(&pageClock,  9, "b0");
NexButton pClock_b1 = NexButton(&pageClock, 10, "b1");
NexButton pClock_b2 = NexButton(&pageClock, 11, "b2");
NexButton pClock_b3 = NexButton(&pageClock, 12, "b3");
NexButton pClock_b4 = NexButton(&pageClock, 13, "b4");

NexScrolltext pClock_SongTitleArtist = NexScrolltext(&pageClock, 15, "g0");


// Page 2 - Setup
NexCrop pSetup_q0 = NexCrop(&pageSetup, 1, "q0");
NexCrop pSetup_q1 = NexCrop(&pageSetup, 3, "q1");

NexButton pSetup_b0 = NexButton(&pageSetup,  2, "b0");
NexButton pSetup_b1 = NexButton(&pageSetup,  4, "b1");
NexButton pSetup_b2 = NexButton(&pageSetup,  5, "b2");
NexButton pSetup_b3 = NexButton(&pageSetup,  6, "b3");
NexButton pSetup_b4 = NexButton(&pageSetup,  7, "b4");


// Page 4 - Upload
NexText pUpload_t0 = NexText(&pageUpload, 1, "t0");
NexText pUpload_t1 = NexText(&pageUpload, 3, "t1");

NexProgressBar pUpload_j0 = NexProgressBar(4, 2, "j0");


// Page 5 - Radio
NexText pRadio_t0 = NexText(&pageRadio,  6, "t0");
NexText pRadio_t1 = NexText(&pageRadio,  7, "t1");
NexText pRadio_t2 = NexText(&pageRadio,  8, "t2");
NexText pRadio_t3 = NexText(&pageRadio, 10, "t3");

NexNumber pRadio_VolDisplay = NexNumber(&pageRadio, 11, "n0");

NexButton pRadio_b0 = NexButton(&pageRadio, 1, "b0");
NexButton pRadio_b1 = NexButton(&pageRadio, 2, "b1");
NexButton pRadio_b2 = NexButton(&pageRadio, 3, "b2");
NexButton pRadio_b3 = NexButton(&pageRadio, 4, "b3");
NexButton pRadio_b4 = NexButton(&pageRadio, 5, "b4");
NexButton pRadio_b5 = NexButton(&pageRadio, 9, "b5");

NexVariable pRadio_state = NexVariable(5, 12, "state");



NexTouch *nex_listen_list[] =
{
//  &pClock_b0,
//  &pClock_b1,
//  &pClock_b2,
  &pClock_b3,
//  &pClock_b4,

  &pSetup_q0,
  &pSetup_q1,
  //&p2b0,

  &pRadio_b0,
  &pRadio_b1,
  &pRadio_b2,
  &pRadio_b3,
  &pRadio_b5,
  NULL
};


bool NexClockInit(int8_t rxPin, int8_t txPin)
{
    bool ret1 = false;
    bool ret2 = false;
    
    nexSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
    while (!nexSerial)
      delay(10);

    sendCommand("");
    sendCommand("bkcmd=1");
    ret1 = recvRetCommandFinished(100);
    sendCommand("page 0");
    ret2 = recvRetCommandFinished(100);
    return (ret1 && ret2);


/*
    if (ret1 && ret2)
    {
      // Change the baud to 115200
      sendCommand("baud 115200");

      // Set the new speed
      nexSerial.updateBaudRate(115200);

      sendCommand("");
      sendCommand("bkcmd=1");
      ret1 = recvRetCommandFinished(100);
      sendCommand("page 0");
      ret2 = recvRetCommandFinished(100);

      return (ret1 && ret2);
    }

    return 0;
*/
}


void pClock_b0PushCallback(void *ptr)
{
  log_i("Alarm 1 pressed\n");
}


void pClock_b1PushCallback(void *ptr)
{
  log_i("Alarm 2 pressed\n");
}


void pClock_b3PushCallback(void *ptr)
{
  playRadio(!ClockSettings.bRadioOn);
}


void NexClockAttachCallbacks()
{
    pClock_b0.attachPush(pClock_b0PushCallback);
    pClock_b1.attachPush(pClock_b1PushCallback);
    pClock_b3.attachPush(pClock_b3PushCallback);

    pSetup_q0.attachPush(pSetup_q0PushCallback);
    pSetup_q1.attachPush(pSetup_q1PushCallback);

    pRadio_b0.attachPush(pRadio_b0PushCallback);
    pRadio_b1.attachPush(pRadio_b1PushCallback);
    pRadio_b2.attachPush(pRadio_b2PushCallback);
    pRadio_b3.attachPush(pRadio_b3PushCallback);
    pRadio_b5.attachPush(pRadio_b5PushCallback);
}


void setUploadProgressBar(const char *objectName, unsigned int val)
{
    String cmd;
    cmd = "Upload.";
    cmd += objectName;
    cmd += ".val=";
    cmd += String(val);
    sendCommand(cmd.c_str());
}


void setSetupToggle(const char *objectName, unsigned int val)
{
    String cmd;
    cmd = "Setup.";
    cmd += objectName;
    cmd += ".picc=";
    cmd += String(val);
    sendCommand(cmd.c_str());
}
