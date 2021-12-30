/**
 * @file NexDualStateButton.h
 *
 * The definition of class NexDSButton. 
 *
 * @author  huang xianming (email:<xianming.huang@itead.cc>)
 * @date    2015/11/11
 *   
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __NEXDSBUTTON_H__
#define __NEXDSBUTTON_H__

#include "NexObject.h"
#include "NexHardware.h"
#include "NexPage.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexDSButton component. 
 *
 * Commonly, you want to do something after push and pop it. It is recommanded that only
 * call @ref NexObject::attachPop to satisfy your purpose. 
 * 
 * @warning Please do not call @ref NexObject::attachPush on this component, even though you can. 
 */
class NexDSButton: public NexObject
{
public: /* methods */
    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexDSButton(NexPage* page, uint8_t cid, const char *name)
        : NexObject(page, cid, name) { }

    uint32_t Get_state0_color_bco0(uint32_t *number) { return getAttrNumber("bco0", number); }
    bool Set_state0_color_bco0(uint32_t number) { return setAttrNumber("bco0", number); }
    uint32_t Get_state1_color_bco1(uint32_t *number) { return getAttrNumber("bco1", number); }
    bool Set_state1_color_bco1(uint32_t number) { return setAttrNumber("bco1", number); }
    uint32_t Get_state0_crop_picc0(uint32_t *number) { return getAttrNumber("picc0", number); }
    bool Set_state0_crop_picc0(uint32_t number) { return setAttrNumber("picc0", number); }
    uint32_t Get_state1_crop_picc1(uint32_t *number) { return getAttrNumber("picc1", number); }
    bool Set_state1_crop_picc1(uint32_t number) { return setAttrNumber("picc1", number); }
    uint32_t Get_state0_image_pic0(uint32_t *number) { return getAttrNumber("pic0", number); }
    bool Set_state0_image_pic0(uint32_t number) { return setAttrNumber("pic0", number); }
    uint32_t Get_state1_image_pic1(uint32_t *number) { return getAttrNumber("pic1", number); }
    bool Set_state1_image_pic1(uint32_t number) { return setAttrNumber("pic1", number); }
};
/**
 * @}
 */



#endif /* #ifndef __NEXDSBUTTON_H__ */
