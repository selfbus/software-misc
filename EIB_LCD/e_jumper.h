/** \file e_jumper.h
 *  \brief Constants and definitions for the page element jumper
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _E_JUMPER_H_
#define _E_JUMPER_H_

#include "System.h"


typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint16_t	picture_index_up;
uint16_t	picture_index_down;
uint16_t	x_pos;
uint16_t	y_pos;
uint8_t		target;
uint16_t	sound_index_up;
uint16_t	sound_index_down;
uint8_t		auto_jump_delay;
} _E_JUMPER_t;

#define SYSTEM_PAGES		2
#define SYSTEM_PAGE_SETUP	0
#define SYSTEM_PAGE_LOCK	1


// draw jumper on screen
void draw_jumper_element (char*);

// touch jumper on screen
uint8_t touch_jumper_element (char*, t_touch_event*, uint8_t*);

// check jumper for auto page change. Returns 1, if jump has been done.
uint8_t auto_check_jumper_element (char*, uint8_t);


#endif // _E_JUMPER_H_
