/** \file e_picture.h
 *  \brief Constants and definitions for the page element Picture
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _E_PICTURE_H_
#define _E_PICTURE_H_

#include "System.h"


typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint16_t	picture_index;
uint16_t	x_pos;
uint16_t	y_pos;
} _E_PICTURE_t;


// draw picture on screen
void draw_picture_element (char*);


#endif // _E_PICTURE_H_
