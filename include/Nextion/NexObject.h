/**
 * @file NexObject.h
 *
 * The definition of class NexObject. 
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
#ifndef __NEXOBJECT_H__
#define __NEXOBJECT_H__
#include <Arduino.h>
#include "NexConfig.h"
#include "NexPage.h"
/**
 * @addtogroup CoreAPI 
 * @{ 
 */

/**
 * Push touch event occuring when your finger or pen coming to Nextion touch pannel. 
 */
#define NEX_EVENT_PUSH  (0x01)

/**
 * Pop touch event occuring when your finger or pen leaving from Nextion touch pannel. 
 */
#define NEX_EVENT_POP   (0x00)  

/**
 * Type of callback funciton when an touch event occurs. 
 * 
 * @param ptr - user pointer for any purpose. Commonly, it is a pointer to a object. 
 * @return none. 
 */
typedef void (*NexObjectEventCb)(void *ptr);


/**
 * Root class of all Nextion components. 
 *
 * Provides the essential attributes of a Nextion component and the methods accessing
 * them. At least, Page ID(pid), Component ID(pid) and an unique name are needed for
 * creating a component in Nexiton library. 
 */
class NexObject 
{
public: /* methods */

    /**
     * Constructor. 
     *
     * @param pid - page id. 
     * @param cid - component id.    
     * @param name - pointer to an unique name in range of all components. 
     */
    NexObject(uint8_t pid, uint8_t cid, const char *name);
    NexObject(NexPage* page, uint8_t cid, const char *name);

    /**
     * Print current object'address, page id, component id and name. 
     *
     * @warning this method does nothing, unless debug message enabled. 
     */
    void printObjInfo(void);

public: /* methods */
    bool getValue(uint32_t *number) { return getAttrNumber("val", number); }
    bool setValue(uint32_t number) { return setAttrNumber("val", number); }
    uint16_t getText(char *buffer, uint16_t len) { return getAttrText("txt", buffer, len); }
    bool setText(const char *buffer) { return setAttrText("txt", buffer); }   
    uint32_t Get_background_color_bco(uint32_t *number) { return getAttrNumber("bco", number); }
    bool Set_background_color_bco(uint32_t number) { return setAttrNumber("bco", number); }
    uint32_t Get_press_background_color_bco2(uint32_t *number) { return getAttrNumber("bco2", number); }
    bool Set_press_background_color_bco2(uint32_t number) { return setAttrNumber("bco2", number); }		
    uint32_t Get_font_color_pco(uint32_t *number) { return getAttrNumber("pco", number); }
    bool Set_font_color_pco(uint32_t number) { return setAttrNumber("pco", number); }
    uint32_t Get_press_font_color_pco2(uint32_t *number) { return getAttrNumber("pco2", number); }
    bool Set_press_font_color_pco2(uint32_t number) { return setAttrNumber("pco2", number); }
    uint32_t Get_place_xcen(uint32_t *number) { return getAttrNumber("xcen", number); }
    bool Set_place_xcen(uint32_t number) { return setAttrNumber("xcen", number); }
    uint32_t Get_place_ycen(uint32_t *number) { return getAttrNumber(".ycen", number); }
    bool Set_place_ycen(uint32_t number) { return setAttrNumber("ycen", number); };			
    uint32_t getFont(uint32_t *number) { return getAttrNumber("font", number); }
    bool setFont(uint32_t number) { return setAttrNumber("font", number); }
    uint32_t Get_background_crop_picc(uint32_t *number) { return getAttrNumber("picc", number); }
    bool Set_background_crop_picc(uint32_t number) { return setAttrNumber("picc", number); }
    uint32_t Get_press_background_crop_picc2(uint32_t *number) { return getAttrNumber("picc2", number); }
    bool Set_press_background_crop_picc2(uint32_t number) { return setAttrNumber("picc2", number); }
    uint32_t Get_background_image_pic(uint32_t *number) { return getAttrNumber("pic", number); }
    bool Set_background_image_pic(uint32_t number) { return setAttrNumber("pic", number); }
    uint32_t Get_press_background_image_pic2(uint32_t *number) { return getAttrNumber("pic2", number); }
    bool Set_press_background_image_pic2(uint32_t number) { return setAttrNumber("pic2", number); }

    bool enable(bool state);

    /*
     * Get page id.
     *
     * @return the id of page.  
     */
    uint8_t getObjPid(void);    

    /*
     * Get component id.
     *
     * @return the id of component.  
     */
    uint8_t getObjCid(void);

    /*
     * Get component name.
     *
     * @return the name of component. 
     */
    const char *getObjName(void);    

    const char* getPageName(void);

    static void iterate(NexObject **list, uint8_t pid, uint8_t cid, int32_t event);

    /**
     * Attach an callback function of push touch event. 
     *
     * @param push - callback called with ptr when a push touch event occurs. 
     * @param ptr - parameter passed into push[default:NULL]. 
     * @return none. 
     *
     * @note If calling this method multiply, the last call is valid. 
     */
    void attachPush(NexObjectEventCb push, void *ptr = NULL);

    /**
     * Detach an callback function. 
     * 
     * @return none. 
     */
    void detachPush(void);

    /**
     * Attach an callback function of pop touch event. 
     *
     * @param pop - callback called with ptr when a pop touch event occurs. 
     * @param ptr - parameter passed into pop[default:NULL]. 
     * @return none. 
     *
     * @note If calling this method multiply, the last call is valid. 
     */
    void attachPop(NexObjectEventCb pop, void *ptr = NULL);

    /**
     * Detach an callback function. 
     * 
     * @return none. 
     */
    void detachPop(void);
    
private: /* methods */ 
    void push(void);
    void pop(void);
    

private: /* data */ 
    uint8_t __pid; /* Page ID */
    uint8_t __cid; /* Component ID */
    const char *__name; /* An unique name */
    NexPage* __page;
    char cmd[64];

    NexObjectEventCb __cb_push;
    void *__cbpush_ptr;
    NexObjectEventCb __cb_pop;
    void *__cbpop_ptr;

protected:
    uint16_t getAttrText(const char* attr, char* buffer, uint16_t len);
    bool setAttrText(const char* attr, const char *buffer);
    
    uint32_t getAttrNumber(const char* attr, uint32_t* number);
    bool setAttrNumber(const char* attr, uint32_t number);
};
/**
 * @}
 */

#endif /* #ifndef __NEXOBJECT_H__ */
