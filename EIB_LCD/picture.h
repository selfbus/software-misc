/** \file picture.h
 *  \brief Constants and definitions for the picture handling
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _PICTURE_H_
#define _PICTURE_H_

#include "System.h"
#include "MemoryMap.h"
#include "tft_io.h"

typedef struct __attribute__ ((packed)) {
uint32_t	offset;
uint16_t	width;
uint16_t	height;
} _PICTURE_DESCRIPTOR_t;

// a picture with this ID is skipped. Added to suppress LED icon outputs for weather symbol display.
#define NO_PICTURE	0xffff

// put picture to lcd (i, x, y)
// i = picture index
// x = starting x-pos
// y = starting y-pos
// returns width of picture
uint16_t  draw_picture (uint16_t, uint16_t, uint16_t);
uint16_t  draw_picture_y (uint16_t, uint16_t, uint16_t);

void set_picture_table_start_address (uint32_t);

void get_picture_size (uint16_t, uint16_t*, uint16_t*);

uint16_t get_picture_width (uint16_t);
uint16_t get_picture_height (uint16_t);

#endif // _PICTURE_H_
