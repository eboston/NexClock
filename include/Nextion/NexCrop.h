/**
 * @file NexCrop.h
 *
 * The definition of class NexCrop. 
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

#ifndef __NEXCROP_H__
#define __NEXCROP_H__

#include "NexObject.h"
//#include "NexHardware.h"
#include "NexPage.h"

/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexCrop component. 
 */
class NexCrop: public NexObject
{
public: /* methods */

    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexCrop(NexPage* page, uint8_t cid, const char *name)
        :NexObject(page, cid, name) {}
};

/**
 * @}
 */

#endif /* #ifndef __NEXCROP_H__ */
