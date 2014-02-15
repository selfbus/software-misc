/** \file page.h
 *  \brief Constants and definitions for the page handling
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _PAGE_H_
#define _PAGE_H_

#include "System.h"
#include "MemoryMap.h"
#include "tft_io.h"
#include "e_picture.h"
#include "e_jumper.h"
#include "e_button.h"
#include "e_sbutton.h"
#include "e_led.h"
#include "e_value.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_count;
char 		page_name[16];
} _PAGE_DESCRIPTOR_t;

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
} _PAGE_ELEMENT_t;

#define	PAGE_ELEMENT_TYPE_PICTURE		0
#define	PAGE_ELEMENT_TYPE_JUMPER		1
#define	PAGE_ELEMENT_TYPE_BUTTON		2
#define	PAGE_ELEMENT_TYPE_LED			3
#define	PAGE_ELEMENT_TYPE_BACKGROUND	4
#define	PAGE_ELEMENT_TYPE_VALUE			5
#define	PAGE_ELEMENT_TYPE_SBUTTON		6


typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		blue;
uint8_t		green;
uint8_t		red;
} _E_BACKGROUND_t;


// moves the page descriptions from Flash into RAM
// returns 0 if ok
// returns 1 on checksum error
uint8_t move_page_descriptions (uint32_t, uint32_t);

// set page active and redraw screen contents
void set_page (uint8_t);

// set_page of previously active page
void recover_active_page (void);

// checks active page components on touch event
void page_touch_event (t_touch_event*);

// check page on EIB event
void lcd_page_process_msg (uint16_t);

// get page descriptor
char* get_page_descriptor (uint8_t);

// process cyclic events
void process_cyclic_page_events (void);

// get the active page ID
uint8_t get_active_page (void);

#endif // _PAGE_H_
