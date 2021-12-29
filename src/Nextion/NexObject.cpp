/**
 * @file NexObject.cpp
 *
 * The implementation of class NexObject. 
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/8/13
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#include "Nextion\NexObject.h"

NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name)
{
    this->__pid = pid;
    this->__cid = cid;
    this->__name = name;
    this->__page = NULL;
}


NexObject::NexObject(NexPage* page, uint8_t cid, const char *name)
{
    this->__pid = page->getObjPid();
    this->__cid = cid;
    this->__name = name;
    this->__page = page;
}


uint16_t NexObject::getAttrText(const char* attr, char* buffer, uint16_t len)
{
    snprintf(cmd, sizeof(cmd), "get %s.%s.%s", __page->getObjName(), getObjName(), attr);
    sendCommand(cmd);
    return recvRetString(buffer,len);
}


bool NexObject::setAttrText(const char* attr, const char *buffer)
{
    snprintf(cmd, sizeof(cmd), "%s.%s.%s=\"%s\"", __page->getObjName(), getObjName(), attr, buffer);
    sendCommand(cmd);
    return recvRetCommandFinished();
}



uint32_t NexObject::getAttrNumber(const char* attr, uint32_t* number)
{
    snprintf(cmd, sizeof(cmd), "get %s.%s.%s", __page->getObjName(), getObjName(), attr);
    sendCommand(cmd);
    return recvRetNumber(number);
}


bool NexObject::setAttrNumber(const char* attr, uint32_t number)
{
    snprintf(cmd, sizeof(cmd), "%s.%s.%s=%d", __page->getObjName(), getObjName(), attr, number);
    sendCommand(cmd);

    snprintf(cmd, sizeof(cmd), "ref %s.%s", __page->getObjName(), getObjName());
    return recvRetCommandFinished();
}


uint8_t NexObject::getObjPid(void)
{
    return __pid;
}

uint8_t NexObject::getObjCid(void)
{
    return __cid;
}

const char* NexObject::getObjName(void)
{
    return __name;
}

const char* NexObject::getPageName(void)
{
    if (__page)
        return __page->getObjName();

    return NULL;
}

void NexObject::printObjInfo(void)
{
    dbSerialPrint("[");
    dbSerialPrint((uint32_t)this);
    dbSerialPrint(":");
    dbSerialPrint(__pid);
    dbSerialPrint(",");
    dbSerialPrint(__cid);
    dbSerialPrint(",");
    if (__name)
    {
        dbSerialPrint(__name);
    }
    else
    {
        dbSerialPrint("(null)");
    }
    dbSerialPrintln("]");
}

