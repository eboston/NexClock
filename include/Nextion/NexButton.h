/**
 * @file NexButton.h
 *
 * The definition of class NexButton. 
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

#ifndef __NEXBUTTON_H__
#define __NEXBUTTON_H__

#include "NexObject.h"
#include "NexPage.h"

/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexButton component. 
 *
 * Commonly, you want to do something after push and pop it. It is recommanded that only
 * call @ref NexObject::attachPop to satisfy your purpose. 
 * 
 * @warning Please do not call @ref NexObject::attachPush on this component, even though you can. 
 */
class NexButton: public NexObject
{
public: /* methods */

    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexButton(NexPage* page, uint8_t cid, const char *name)
        : NexObject(page, cid, name) {}
};
/**
 * @}
 */


#endif /* #ifndef __NEXBUTTON_H__ */
