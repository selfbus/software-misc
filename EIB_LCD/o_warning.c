/** \file o_warning.c
 *  \brief Functions for the event support
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	The objects provided by this module are processed on reception of an EIB message. 
 *	They are not bound to any display page.
 *
 *	Implemented functions:
 *	- automatic page change
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "o_warning.h"
#include "System.h"

void check_warning_object (char* cp, uint8_t obj) {

_O_WARNING_t*	p;
uint8_t		eib_value;

	p = (_O_WARNING_t*) cp;

	if (p->warning_object_id == obj) {
		// get new value
		eib_value = eib_get_object_8_value (p->warning_object_id);
		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

		if (eib_value) {
			if (get_active_page() != p->destination_page)
				set_page (p->destination_page);
		}
	}
}
