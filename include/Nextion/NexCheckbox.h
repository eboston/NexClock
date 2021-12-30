/**
 * @file NexCheckbox.h
 *
 * The definition of class NexCheckbox. 
 *
 * @author huang xiaoming (email:<xiaoming.huang@itead.cc>)
 * @date 2016/9/13
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __NEXCHECKBOX_H__
#define __NEXCHECKBOX_H__

#include "NexObject.h"
#include "NexHardware.h"
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
class NexCheckbox: public NexObject
{
public: /* methods */

    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexCheckbox(NexPage* page, uint8_t cid, const char *name)
        : NexObject(page, cid, name) { }
};
/**
 * @}
 */


#endif /* #ifndef __NEXCHECKBOX_H__ */