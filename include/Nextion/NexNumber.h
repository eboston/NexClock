/**
 * @file NexNumber.h
 *
 * The definition of class NexNumber. 
 *
 * @author Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date 2015/8/13
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
 
#ifndef __NEXNUMBER_H__
#define __NEXNUMBER_H__

#include "Nexobject.h"
#include "NexHardware.h"
#include "NexPage.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexNumber component.
 */
class NexNumber: public NexObject
{
public: /* methods */
    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexNumber(NexPage* page, uint8_t cid, const char *name)
        : NexObject(page, cid, name) { }

};

/**
 * @}
 */

#endif /* #ifndef __NEXNUMBER_H__ */
