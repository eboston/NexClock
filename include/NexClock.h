#if !defined(NEXCLOCK_H)
#define NEXCLOCK_H

#include "Nextion/nextion.h"

extern NexPage pageStartup;
extern NexPage pageClock;
extern NexPage pageSetup;
extern NexPage pageAlarm;
extern NexPage pageUpload;
extern NexPage pageRadio;

extern NexText pStartup_t0;
extern NexText pStartup_t1;
extern NexText pStartup_t2;
extern NexText pStartup_t3;
extern NexGauge pStartup_z0;
extern NexVariable pStartup_radioState;


extern NexText pClock_t0;
extern NexText pClock_t1;
extern NexText pClock_t2;
extern NexText pClock_t3;
extern NexText pClock_t4;
extern NexText pClock_t5;
extern NexText pClock_t6;
extern NexText pClock_t7;

extern NexText pUpload_t0;
extern NexText pUpload_t1;
extern NexProgressBar pUpload_j0;

extern NexButton pClock_b0;
extern NexButton pClock_b1;
extern NexButton pClock_b2;
extern NexButton pClock_b3;
extern NexButton pClock_b4;

extern NexButton pSetup_b0;
extern NexButton pSetup_b1;
extern NexButton pSetup_b2;
extern NexButton pSetup_b3;
extern NexButton pSetup_b4;
extern NexCrop pSetup_q0;
extern NexCrop pSetup_q1;

extern NexButton pRadio_b0;
extern NexButton pRadio_b1;
extern NexButton pRadio_b2;
extern NexButton pRadio_b3;
extern NexButton pRadio_b4;
extern NexButton pRadio_b5;
extern NexText pRadio_t0;
extern NexText pRadio_t1;
extern NexText pRadio_t2;
extern NexText pRadio_t3;
extern NexNumber pRadio_VolDisplay;
extern NexVariable pRadio_state;



extern NexTouch *nex_listen_list[];


void NexClockAttachCallbacks();

bool NexClockInit(int8_t rxPin, int8_t txPin);
void NexClockLoop();

//void setText(NexPage page, const char *objectName, const char *buffer);
//void setText(NexPage page, NexText textObj, const char *buffer);

//uint16_t getText(NexPage page, const char *objectName, char *buffer, uint16_t len);
//bool  setTextColor(NexPage page, const char *objectName, uint32_t number);

void setUploadProgressBar(const char *objectName, unsigned int val);
void setSetupToggle(const char *objectName, unsigned int val);

//bool setButtonBackgroundColor(NexPage page, NexButton button, uint32_t number);
//bool setButtonFontColor(NexPage page, NexButton button, uint32_t number);


#endif
