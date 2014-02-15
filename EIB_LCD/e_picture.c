/** \file e_picture.c
 *  \brief Functions for static picture element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display fixed picture element on LCD page
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "e_picture.h"

void draw_picture_element (char* cp) {

_E_PICTURE_t*	p;

	p = (_E_PICTURE_t*) cp;

	draw_picture (p->picture_index, p->x_pos, p->y_pos);

}
