#ifndef __NEXPAGE_H__
#define __NEXPAGE_H__

#include <Arduino.h>

bool recvRetNumber(uint32_t *number, uint32_t timeout = 100);
uint16_t recvRetString(char *buffer, uint16_t len, uint32_t timeout = 100);
void sendCommand(const char* cmd);
bool recvRetCommandFinished(uint32_t timeout = 100);


class NexPage
{
public:
    NexPage(uint8_t pid, uint8_t cid, const char *name);
    
    bool show(void);

    uint8_t getObjPid() { return __pid; }
    const char* getObjName() { return __name; }

private:
    uint8_t __pid;
    const char* __name;
};

#endif /* #ifndef __NEXPAGE_H__ */
