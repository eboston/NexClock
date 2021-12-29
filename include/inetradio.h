#if !defined(__INETRADIO_H_)
#define __INETRADIO_H_

//#define STATION_NUM  9

extern TaskHandle_t playMusicTaskHandle;
extern TaskHandle_t handleIncomingRadioDataTaskHandle;
void handleIncomingRadioData();

void mp3Start();
void handleIncomingRadioDataTask(void *parameter = NULL);
void playRadio(bool state, bool saveChange = true);

void pRadio_b0PushCallback(void *ptr);
void pRadio_b1PushCallback(void *ptr);
void pRadio_b2PushCallback(void *ptr);
void pRadio_b3PushCallback(void *ptr);
void pRadio_b5PushCallback(void *ptr);




#endif  // __INETRADIO_H_
