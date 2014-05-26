/** \file listen.h
 *  \brief Constants and definitions for always listening elements
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	This module contains functions for supporting "always listening" objects, 
 *	which continously monitor the EIB Messages independent from any displayed page.
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _LISTEN_H_
#define _LISTEN_H_

#include "System.h"
#include "MemoryMap.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_count;
uint8_t		checksum;
} _LISTEN_DESCRIPTOR_t;

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
} _LISTEN_ELEMENT_t;

#define	LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE		0
#define	LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE	1
#define LISTEN_ELEMENT_TYPE_LED					2
#define	LISTEN_ELEMENT_TYPE_WARNING				3

// divider for 30ms -> 120ms
#define LISTEN_OBJECTS_TIMER_MAX				4

// moves the page descriptions from Flash into RAM
// returns 0 if ok
// returns 1 on checksum error
uint8_t move_listen_descriptions (uint32_t, uint32_t);

// check page on EIB event
void lcd_listen_process_msg (uint16_t);

// process new timer event
void lcd_listen_timer_event (void);
// init hardware of listen objects
void lcd_init_listen_objects (void);


#endif // _LISTEN_H_
