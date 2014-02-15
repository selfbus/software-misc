/** \file o_led.h
 *  \brief Constants and definitions for the LED page element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _O_LED_H_
#define _O_LED_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		hw_object_id;
uint8_t		eib_object_listen;
// Parameter: d5, d4: Flash frequency, d3, d2: state@1, d1, d0: state@0
uint8_t		parameter;
} _O_LED_t;
#define BACKLIGHT_PARAMETER_LISTEN 		0x01

// check for update from EIB
void check_led_object (char*, uint8_t);

// init object
void init_led_object (char*);


#endif // _O_LED_H_