#include "Nextion\NexPage.h"

NexPage::NexPage(uint8_t pid, uint8_t cid, const char *name)
    : __pid(pid)
    , __name(name)
{
}

bool NexPage::show(void)
{
    const char *name = getObjName();
    if (!name)
        return false;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "page %s", name);
    sendCommand(cmd);

    return recvRetCommandFinished();
}

