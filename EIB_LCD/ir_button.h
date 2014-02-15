/** \file e_button.h
 *  \brief Constants and definitions for the control of external buttons
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _IR_BUTTON_H_
#define _IR_BUTTON_H_

#include "System.h"


typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		hardware_id;
uint8_t		rc5_address;
uint8_t		rc5_command;
uint8_t		eib_function;
uint8_t		eib_object0;
uint8_t		eib_object1;
uint8_t		value[2];
} _IR_BUTTON_t;

// touch button on screen
void ir_button_pressed (char *cp, uint8_t evt);

#endif // _IR_BUTTON_H_
