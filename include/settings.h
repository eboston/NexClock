#if !defined(SETTINGS_H)
#define SETTINGS_H

typedef struct Settings_struct
{
    bool        b24Hour;
    bool        bFlashColon;
    uint8_t     lastRadioStation;
    bool        bRadioOn;
    char        szSSID[16];
    char        szPWD[16];
    uint8_t     volumeLevel;
    bool        bMetadata;
//    NexPage*    plastPage;
} Settings;

class NexCrop;


// Setup values
extern bool  b24Hour;
extern bool  bFlashColon;
extern Settings ClockSettings;
extern NexCrop pSetup_q0;

void listDir(const char * dirname);
bool StartLITTLEFS();

void dumpSettings();
bool writeSettings();
bool readSettings();
void ApplySettingToPage();

void pSetup_FlashColon_PushCallback(void *ptr);
void pSetup_24Hour_PushCallback(void *ptr);
void pSetup_Metadata_PushCallback(void *ptr);



#endif
