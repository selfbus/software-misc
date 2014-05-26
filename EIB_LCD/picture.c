/** \file picture.c
 *  \brief Functions to display picture objects
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display pictures stored in the Nand Flash
 *	- return picture dimensions to support text output
 *
 *	This module supports all page elements, which need to display
 *	pictures (including icons) stored in the Nand Flash.
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "picture.h"

// start address of picture descriptor table in Flash
uint32_t picture_table_start_address;

uint16_t draw_picture (uint16_t i, uint16_t x_pos, uint16_t y_pos) {

uint32_t	pict;
uint32_t	ofs;
uint16_t	width, height;

	if (i == NO_PICTURE)
		return 0;
	// get descriptor of picture i
	pict = sizeof (_PICTURE_DESCRIPTOR_t) * i;
	pict += picture_table_start_address;
	
	// read picture properties from Flash
	ofs = read_flash_abs (pict +2);
	ofs = (ofs << 16) | read_flash_abs (pict);
	width = read_flash_abs (pict+4);
	height = read_flash_abs (pict+6);

	// move image to tft
	tft_put_flash_image (x_pos, y_pos, x_pos + width -1, y_pos + height -1, (picture_table_start_address + ofs) >> 1 );
	return width;
}

uint16_t draw_picture_y (uint16_t i, uint16_t x_pos, uint16_t y_pos) {

uint32_t	pict;
uint32_t	ofs;
uint16_t	width, height;

	// get descriptor of picture i
	pict = sizeof (_PICTURE_DESCRIPTOR_t) * i;
	pict += picture_table_start_address;
	
	// read picture properties from Flash
	ofs = read_flash_abs (pict +2);
	ofs = (ofs << 16) | read_flash_abs (pict);
	width = read_flash_abs (pict+4);
	height = read_flash_abs (pict+6);

	// move image to tft
	tft_put_flash_image (x_pos, y_pos, x_pos + width -1, y_pos + height -1, (picture_table_start_address + ofs) >> 1 );
	return height;
}

void set_picture_table_start_address (uint32_t s) {

	picture_table_start_address = s;
}

void get_picture_size (uint16_t i, uint16_t *width, uint16_t *height) {

uint32_t	pict;

	// get descriptor of picture i
	pict = sizeof (_PICTURE_DESCRIPTOR_t) * i;
	pict += picture_table_start_address;
	
	// read picture properties from Flash
	*width = read_flash_abs (pict+4);
	*height = read_flash_abs (pict+6);
}

uint16_t get_picture_width (uint16_t i) {

uint32_t	pict;

	// get descriptor of picture i
	pict = sizeof (_PICTURE_DESCRIPTOR_t) * i;
	pict += picture_table_start_address;
	
	// read picture properties from Flash
	return read_flash_abs (pict+4);
}

uint16_t get_picture_height (uint16_t i) {

uint32_t	pict;

	// get descriptor of picture i
	pict = sizeof (_PICTURE_DESCRIPTOR_t) * i;
	pict += picture_table_start_address;
	
	// read picture properties from Flash
	return read_flash_abs (pict+6);
}
