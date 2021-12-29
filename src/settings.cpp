#include <Arduino.h>
#include <FS.h>
#include <LITTLEFS.h>

#include "main.h"

#include "NexClock.h"
#include "settings.h"

#define CONFIG  "/conf"
#define FORMAT_FS_IF_FAILED true

bool bFSStarted = false;


// Setup values
Settings ClockSettings =
{
    false,          // 24 Hour
    true,           // Flash Colon
    0,              // Last Radio Station
    false,          // Radio on
    "Oxford-HASP",  // SSID
    "HermanHASP",   // Password
    80,             // Volume Level
};


void dumpSettings()
{
    log_w("Settings:");
    log_w("       24 Hour = %d", ClockSettings.b24Hour);
    log_w("   Flash Colon = %d", ClockSettings.bFlashColon);
    log_w("  Last Station = %d", ClockSettings.lastRadioStation);
    log_w("      Radio On = %d", ClockSettings.bRadioOn);
    log_w("          SSID = '%s'", ClockSettings.szSSID);
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
    if (!bFSStarted)
    {
        Serial.printf("File System has not been started yet.\n");
        return;
    }
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels)
            {
                listDir(fs, file.name(), levels -1);
            }
        } 
        else 
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


void listDir(const char * dirname)
{
    listDir(LITTLEFS, dirname, 0);
}


void ApplySettingToPage()
{
    setSetupToggle("q0", ClockSettings.bFlashColon);
    setSetupToggle("q1", ClockSettings.b24Hour);
}


bool writeSettings()
{
    if (!bFSStarted)
    {
        log_w("File System has not been started yet.");
        return false;
    }

    File f = LITTLEFS.open(CONFIG, "w");
    if (!f) 
    {
        log_w("writeSettingsStruct: File open failed!");
        return false;
    } 

    if (f.write((const uint8_t *)&ClockSettings, sizeof(ClockSettings)) != sizeof(ClockSettings))
    {
        log_w("Error writing settings struct");
        return false;
    }

    return true;
}


bool readSettings()
{
    if (!bFSStarted)
    {
        log_w("File System has not been started yet.");
        return false;
    }

    if (LITTLEFS.exists(CONFIG) == false) 
    {
        log_w("readSettings: Settings File does not yet exists.");
        return writeSettings();
    }

    File f = LITTLEFS.open(CONFIG, "r");
    f.readBytes((char *)&ClockSettings, sizeof(ClockSettings));

    return true;
}


bool StartLITTLEFS()
{
  if (!LITTLEFS.begin(FORMAT_FS_IF_FAILED))
  {
    log_w("setup: LITTLEFS Mount Failed\n");
    return false;
  }
  log_w("LITTLEFS mounted\n");
  bFSStarted = true;

//  LITTLEFS.remove(CONFIG);

  return true;
}


void pSetup_q0PushCallback(void *ptr)
{
  uint32_t number = 0;

  pSetup_q0.Get_background_crop_picc(&number);

  number += 1;
  number %= 2;

  pSetup_q0.Set_background_crop_picc(number);

  ClockSettings.bFlashColon = number;
  writeSettings();
}


void pSetup_q1PushCallback(void *ptr)
{
  uint32_t number = 0;

  pSetup_q1.Get_background_crop_picc(&number);

  number += 1;
  number %= 2;

  pSetup_q1.Set_background_crop_picc(number);

  ClockSettings.b24Hour = number;
  writeSettings();
}

