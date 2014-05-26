/** \file o_backlight.c
 *  \brief This module contains the backlight control element functions
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	The objects provided by this module are processed on reception of an EIB message. 
 *	They are not bound to any display page.
 *
 *	Implemented functions:
 *	- backlight intensitiy control by EIB objects
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */

#include "o_backlight.h"
#include "System.h"

void check_backlight_object (char* cp, int obj) {

_O_BACKLIGHT_t*	p;
uint8_t			eib_value;

	p = (_O_BACKLIGHT_t*) cp;

	if (!(p->parameter & BACKLIGHT_PARAMETER_LISTEN))
		return;

	if (p->eib_object_listen == obj) {
		// set new value
		eib_value = eib_get_object_8_value (p->eib_object_listen);
		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

		if (p->element_type == LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE) {

			backlight_dimming = eib_value;
			refresh_backlight ();
		}
	
		if (p->element_type == LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE) {

			backlight_active = eib_value;
			refresh_backlight ();
		}
	}
}


// init object
void init_backlight_object (char* cp) {

_O_BACKLIGHT_t*	p;
	p = (_O_BACKLIGHT_t*) cp;

	if (p->element_type == LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE) {

		backlight_dimming = p->default_value;
		refresh_backlight ();
	}
	
	if (p->element_type == LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE) {

		backlight_active = p->default_value;
		refresh_backlight ();
	}

}
