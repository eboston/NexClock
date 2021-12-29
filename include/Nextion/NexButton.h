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

#include "NexTouch.h"
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
 * call @ref NexTouch::attachPop to satisfy your purpose. 
 * 
 * @warning Please do not call @ref NexTouch::attachPush on this component, even though you can. 
 */
class NexButton: public NexTouch
{
public: /* methods */

    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexButton(NexPage* page, uint8_t cid, const char *name)
        : NexTouch(page, cid, name) {}


    /**
     * Get text attribute of component.
     *
     * @param buffer - buffer storing text returned. 
     * @param len - length of buffer. 
     * @return The real length of text returned. 
     */
    uint16_t getText(char *buffer, uint16_t len) { return getAttrText("txt", buffer, len); }

    /**
     * Set text attribute of component.
     *
     * @param buffer - text buffer terminated with '\0'. 
     * @return true if success, false for failure. 
     */
    bool setText(const char *buffer) { return setAttrText("txt", buffer); }   

    /**
     * Get bco attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_background_color_bco(uint32_t *number) { return getAttrNumber("bco", number); }
	
    /**
     * Set bco attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_background_color_bco(uint32_t number) { return setAttrNumber("bco", number); }
	
    /**
     * Get bco2 attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_press_background_color_bco2(uint32_t *number) { return getAttrNumber("bco2", number); }

    /**
     * Set bco2 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_press_background_color_bco2(uint32_t number) { return setAttrNumber("bco2", number); }		
	
    /**
     * Get pco attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_font_color_pco(uint32_t *number) { return getAttrNumber("pco", number); }
	
    /**
     * Set pco attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_font_color_pco(uint32_t number) { return setAttrNumber("pco", number); }
	
    /**
     * Get pco2 attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_press_font_color_pco2(uint32_t *number) { return getAttrNumber("pco2", number); }
	
    /**
     * Set pco2 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_press_font_color_pco2(uint32_t number) { return setAttrNumber("pco2", number); }
	
    /**
     * Get xcen attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_place_xcen(uint32_t *number) { return getAttrNumber("xcen", number); }
	
    /**
     * Set xcen attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_place_xcen(uint32_t number) { return setAttrNumber("xcen", number); }

    /**
     * Get ycen attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_place_ycen(uint32_t *number) { return getAttrNumber(".ycen", number); }

    /**
     * Set ycen attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_place_ycen(uint32_t number) { return setAttrNumber("ycen", number); };			
	
    /**
     * Get font attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t getFont(uint32_t *number) { return getAttrNumber("font", number); }
	
    /**
     * Set font attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool setFont(uint32_t number) { return setAttrNumber("font", number); }

    /**
     * Get picc attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_background_cropi_picc(uint32_t *number) { return getAttrNumber("picc", number); }

    /**
     * Set picc attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_background_crop_picc(uint32_t number) { return setAttrNumber("picc", number); }

    /**
     * Get picc2 attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_press_background_crop_picc2(uint32_t *number) { return getAttrNumber("picc2", number); }
	
    /**
     * Set picc2 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_press_background_crop_picc2(uint32_t number) { return setAttrNumber("picc2", number); }

    /**
     * Get pic attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */
    uint32_t Get_background_image_pic(uint32_t *number) { return getAttrNumber("pic", number); }

    /**
     * Set pic attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_background_image_pic(uint32_t number) { return setAttrNumber("pic", number); }

    /**
     * Get pic2 attribute of component
     *
     * @param number - buffer storing data return
     * @return the length of the data 
     */	
    uint32_t Get_press_background_image_pic2(uint32_t *number) { return getAttrNumber("pic2", number); }

    /**
     * Set pic2 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_press_background_image_pic2(uint32_t number) { return setAttrNumber("pic2", number); }
};
/**
 * @}
 */


#endif /* #ifndef __NEXBUTTON_H__ */
