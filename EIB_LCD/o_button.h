/** \file o_button.h
 *  \brief Constants and definitions for the button page element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _O_BUTTON_H_
#define _O_BUTTON_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		hw_object_id;
uint8_t		eib_object_send;
uint8_t		function;
uint8_t		interval;
// space for object variables
uint8_t		state; // d7=current state, d6:d0: counter [s] for repetitions
uint8_t		filter;
} _O_BUTTON_t;

#define HARDWARE_BUTTON_TOGGLE			0x01
#define HARDWARE_BUTTON_SEND_0			0x02
#define HARDWARE_BUTTON_REPEAT_SEND_0	0x06
#define HARDWARE_BUTTON_SEND_1			0x03
#define HARDWARE_BUTTON_REPEAT_SEND_1	0x07

// cyclic check of button state
void check_hardware_button (char*);

// init object
void init_button_object (char*);


#endif // _O_BUTTON_H_
