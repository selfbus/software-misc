/** \file e_led.h
 *  \brief Constants and definitions for the page element LED
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _E_LED_H_
#define _E_LED_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint16_t	picture_off_index;
uint16_t	picture_on_index;
uint16_t	picture_warning_index;
uint16_t	x_pos;
uint16_t	y_pos;
uint8_t		eib_object_listen;
uint8_t		eib_object_send;
uint8_t		parameter;
uint8_t		repeat_radio_value; // radio button index or warning sound repetitions
uint16_t	sound_index_up;
uint16_t	sound_index_down;
uint16_t	sound_index_warning;
} _E_LED_t;
#define LED_PARAMETER_BITPOS	0x07
#define LED_PARAMETER_WARNING	0x10
#define LED_PARAMETER_RADIO		0x20
#define LED_PARAMETER_SEND 		0x40
#define LED_PARAMETER_INIT 		0x80

// draw picture on screen
void draw_led_element (char*);

// check for update from EIB
void check_led_element (char*, int);

uint8_t touch_led_element (char*, t_touch_event*, uint8_t*);
/* cyclic check of the warning state to toggle the picture */
uint8_t check_led_warning_state (char*, uint8_t);

#endif // _E_LED_H_
