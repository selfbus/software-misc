/** \file o_backlight.h
 *  \brief Constants and definitions for the backlight intensity control element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _O_BACKLIGHT_H_
#define _O_BACKLIGHT_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		default_value;
uint8_t		eib_object_listen;
// d0: 1=listen
uint8_t		parameter;
} _O_BACKLIGHT_t;
#define BACKLIGHT_PARAMETER_LISTEN 		0x01

// check for update from EIB
void check_backlight_object (char*, int);

// init object
void init_backlight_object (char*);


#endif // _O_BACKLIGHT_H_
