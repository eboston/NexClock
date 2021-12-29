/**
 * @file NexText.h
 *
 * The definition of class NexText. 
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
 
#ifndef __NEXTEXT_H__
#define __NEXTEXT_H__

#include "NexTouch.h"
#include "NexHardware.h"
#include "NexPage.h"

/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexText component.
 */
class NexText: public NexTouch
{
public: /* methods */
    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexText(NexPage* page, uint8_t cid, const char *name)
        : NexTouch(page, cid, name) { }
    
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
     * @param number - buffer storing data retur
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
     * Get pco attribute of component
     *
     * @param number - buffer storing data retur
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
     * Get xcen attribute of component
     *
     * @param number - buffer storing data retur
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
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_place_ycen(uint32_t *number) { return getAttrNumber("ycan", number); }

    /**
     * Set ycen attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_place_ycen(uint32_t number) { return setAttrNumber("ycen", number); }
	
    /**
     * Get font attribute of component
     *
     * @param number - buffer storing data retur
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
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_background_crop_picc(uint32_t *number) { return getAttrNumber("picc", number); }

    /**
     * Set picc attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_background_crop_picc(uint32_t number) { return setAttrNumber("picc", number); }
	
    /**
     * Get pic attribute of component
     *
     * @param number - buffer storing data retur
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
    
};

/**
 * @}
 */

#endif /* #ifndef __NEXTEXT_H__ */
