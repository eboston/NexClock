#if !defined(NEXCLOCK_H)
#define NEXCLOCK_H

#include "Nextion/nextion.h"

extern NexPage pageStartup;
extern NexPage pageClock;
extern NexPage pageSetup;
extern NexPage pageAlarm;
extern NexPage pageUpload;
extern NexPage pageRadio;

extern NexText          pStartup_t0;
extern NexText          pStartup_t1;
extern NexText          pStartup_t2;
extern NexText          pStartup_t3;
extern NexGauge         pStartup_z0;
extern NexVariable      pStartup_radioState;


extern NexText          pClock_t0;
extern NexText          pClock_t1;
extern NexText          pClock_t2;
extern NexText          pClock_t3;
extern NexText          pClock_t4;
extern NexText          pClock_t5;
extern NexText          pClock_t6;
extern NexText          pClock_t7;
extern NexButton        pClock_b0;
extern NexButton        pClock_b1;
extern NexButton        pClock_b2;
extern NexButton        pClock_b3;
extern NexButton        pClock_b4;
extern NexScrolltext    pClock_SongTitleArtist;

extern NexButton        pSetup_b0;
extern NexButton        pSetup_b1;
extern NexButton        pSetup_b2;
extern NexButton        pSetup_b3;
extern NexButton        pSetup_b4;
extern NexCrop          pSetup_FlashColon;
extern NexCrop          pSetup_24Hour;
extern NexCrop          pSetup_Metadata;

extern NexText          pUpload_t0;
extern NexText          pUpload_t1;
extern NexProgressBar   pUpload_j0;

extern NexButton        Radio_b0;
extern NexButton        pRadio_b1;
extern NexButton        pRadio_b2;
extern NexButton        pRadio_b3;
extern NexButton        pRadio_b4;
extern NexButton        pRadio_b5;
extern NexText          pRadio_t0;
extern NexText          pRadio_t1;
extern NexText          pRadio_t2;
extern NexText          pRadio_t3;
extern NexText          pRadio_Time;
extern NexNumber        pRadio_VolDisplay;
extern NexVariable      pRadio_state;



extern NexObject *nex_listen_list[];


void NexClockAttachCallbacks();

bool NexClockInit(int8_t rxPin, int8_t txPin);
void NexClockLoop();

void setUploadProgressBar(const char *objectName, unsigned int val);
void setSetupToggle(const char *objectName, unsigned int val);

#endif
