/** \file e_button.h
 *  \brief Constants and definitions for the page element button
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _E_BUTTON_H_
#define _E_BUTTON_H_

#include "System.h"


typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint16_t	picture_index_up;
uint16_t	picture_index_down;
uint16_t	x_pos;
uint16_t	y_pos;
uint8_t		eib_object0;
uint8_t		eib_object1;
uint8_t		eib_function;
uint16_t	sound_index_up;
uint16_t	sound_index_down;
uint8_t		value[2];
int16_t		max;
int16_t		min;
} _E_BUTTON_t;

#define EIB_BUTTON_FUNCTION_TOGGLE		0
#define EIB_BUTTON_FUNCTION_ON			1
#define EIB_BUTTON_FUNCTION_OFF			2

#define EIB_BUTTON_FUNCTION_BRIGHTER	3
#define EIB_BUTTON_FUNCTION_DARKER		4
#define EIB_BUTTON_FUNCTION_ON_BRIGHTER	5
#define EIB_BUTTON_FUNCTION_OFF_DARKER	6

#define EIB_BUTTON_FUNCTION_UP			7
#define EIB_BUTTON_FUNCTION_DOWN		8
#define EIB_BUTTON_FUNCTION_STEPUP		9
#define EIB_BUTTON_FUNCTION_STEPDOWN	10
#define EIB_BUTTON_FUNCTION_UP_STEPUP		11
#define EIB_BUTTON_FUNCTION_DOWN_STEPDOWN	12
#define EIB_BUTTON_FUNCTION_8BIT_VALUE	13
#define EIB_BUTTON_FUNCTION_16BIT_VALUE	14

#define EIB_BUTTON_FUNCTION_DELTA_EIS6	15
#define	EIB_BUTTON_FUNCTION_DELTA_EIS5	16

// draw button on screen
void draw_button_element (char*);

// touch button on screen
uint8_t touch_button_element (char*, t_touch_event*, uint8_t*);

#endif // _E_BUTTON_H_
