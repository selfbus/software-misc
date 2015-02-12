/** \file o_timeout.c
 *  \brief This module contains the object timeout functions
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	The objects provided by this module are processed on reception of an EIB message. 
 *	They are not bound to any display page.
 *
 *	Implemented functions:
 *	- timeout control for EIB objects
 *
 *	Copyright (c) 2011-2015 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */

#include "o_timeout.h"
#include "System.h"

void check_timeout_object (char* cp, uint8_t obj) {

_O_TIMEOUT_t*	p;

	p = (_O_TIMEOUT_t*) cp;

	if (p->eib_object_listen == obj) {
		// clear counter
		p->timeout_counter = 0;
	}
}

// trigger timer with 1sec interval
void tick_timeout_object (char* cp) {

_O_TIMEOUT_t*	p;

	p = (_O_TIMEOUT_t*) cp;

	if (p->timeout_counter == 0xffff)
		return;
	// increment counter
	p->timeout_counter++;

}

// init object timeout counter
void init_timeout_object (char* cp) {

_O_TIMEOUT_t*	p;

	p = (_O_TIMEOUT_t*) cp;
	// clear counter
	p->timeout_counter = 0xffff;
}

// get object timeout counter
uint8_t get_timeout_object_counter (char* cp, uint8_t obj, uint16_t *val) {

_O_TIMEOUT_t*	p;

	p = (_O_TIMEOUT_t*) cp;

	if (p->eib_object_listen == obj) {
		*val = p->timeout_counter;
		return 1;
	}
	return 0;
}


