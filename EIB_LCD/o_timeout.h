/** \file o_led.h
 *  \brief Constants and definitions for the object value timout element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2015 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _O_TIMEOUT_H_
#define _O_TIMEOUT_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		eib_object_listen;
uint16_t	timeout_counter;
} _O_TIMEOUT_t;

// check for update from EIB
void check_timeout_object (char*, uint8_t);
// init object
void init_timeout_object (char*);
// trigger timer with 1sec interval
void tick_timeout_object (char*);
// get timeout counter value
uint8_t get_timeout_object_counter (char*, uint8_t, uint16_t*);
	
#endif // _O_TIMEOUT_H_
