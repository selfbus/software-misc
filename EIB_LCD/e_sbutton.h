/** \file e_sbutton.h
 *  \brief Constants and definitions for the page element state-button
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _E_SBUTTON_H_
#define _E_SBUTTON_H_

#include "System.h"


typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint16_t	picture_index_up_off;
uint16_t	picture_index_down_off;
uint16_t	picture_index_up_on;
uint16_t	picture_index_down_on;
uint16_t	x_pos;
uint16_t	y_pos;
uint8_t		eib_object_send;
uint8_t		eib_object_listen;
uint8_t		eib_function;
uint16_t	sound_index_up;
uint16_t	sound_index_down;
} _E_SBUTTON_t;

#define EIB_SBUTTON_FUNCTION_TOGGLE		0
#define EIB_SBUTTON_FUNCTION_ON			1
#define EIB_SBUTTON_FUNCTION_OFF		2

// draw button on screen
void draw_sbutton_element (char*, uint8_t);

// touch button on screen
uint8_t touch_sbutton_element (char*, t_touch_event*, uint8_t*);
// check for update from EIB
// pointer to button descriptor
// object#
// boolean: this element is touched
// state of touched element
void check_sbutton_element (char*, int, uint8_t, uint8_t);

#endif // _E_SBUTTON_H_
