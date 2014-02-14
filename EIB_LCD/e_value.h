/** \file e_value.h
 *  \brief Constants and definitions for the page element nummeric value
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _E_VALUE_H_
#define _E_VALUE_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint16_t	picture_index1;
uint16_t	x_pos;
uint16_t	y_pos;
uint8_t		text_x;
uint8_t		chars;
uint8_t		eib_object_listen;
// d1: 1=init
uint8_t		parameter;
} _E_VALUE_t;
#define VALUE_PARAMETER_INIT 0x02

#define MAX_VALUE_LENGTH 14

#define PICTURE_OFFSET_BACKGROUND	0
#define PICTURE_OFFSET_ZERO			1
#define PICTURE_OFFSET_POSTFIXUNIT	15
// "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "+", "-", ":"
#define CHAR_OFFSET_DOT		10
#define CHAR_OFFSET_PLUS	11
#define CHAR_OFFSET_MINUS	12
#define CHAR_OFFSET_DP		13

// draw picture on screen
void draw_value_element (char*);

// check for update from EIB
void check_value_element (char*, int);

#endif // _E_VALUE_H_
