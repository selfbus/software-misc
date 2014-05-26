/** \file o_led.c
 *  \brief Functions for the support of external LEDs
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	The objects provided by this module are processed on reception of an EIB message and
 *	cyclicly triggered by a timer.
 *	They are not bound to any display page.
 *
 *	Implemented functions:
 *	- set state of external LED according to EIB objects
 *	- flash external LED according to EIB object status and LED parameters
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */

#include "o_led.h"
#include "System.h"

void check_led_object (char* cp, uint8_t tflags) {

_O_LED_t*	p;
uint8_t			eib_value;

	p = (_O_LED_t*) cp;

	// get new value
	eib_value = eib_get_object_8_value (p->eib_object_listen);
	XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

	// get led output value for led state
uint8_t lc; // led config
uint8_t ls; // led state
uint8_t lf; // led frequency
	if (eib_value)
		lc = (p->parameter >> 2) & 0x03;
	else 
		lc = p->parameter & 0x03;
	
	if (lc & 0x02) {
		// led flashes
		lf = (p->parameter >> 4) & 0x03;
		ls = ((8 >> lf) & tflags);
	}
	else {
		ls = lc;
	}


	switch (p->hw_object_id) {

		case HARDWARE_OBJECT_PE0:
			if (ls)
				SET1_HARDWARE_OBJECT_PE0
			else 
				SET0_HARDWARE_OBJECT_PE0
		break;
		case HARDWARE_OBJECT_PE1:
			if (ls)
				SET1_HARDWARE_OBJECT_PE1
			else 
				SET0_HARDWARE_OBJECT_PE1
		break;
		case HARDWARE_OBJECT_PE2:
			if (ls)
				SET1_HARDWARE_OBJECT_PE2
			else 
				SET0_HARDWARE_OBJECT_PE2
		break;
		case HARDWARE_OBJECT_PF4:
			if (ls)
				SET1_HARDWARE_OBJECT_PF4
			else 
				SET0_HARDWARE_OBJECT_PF4
		break;
		case HARDWARE_OBJECT_PF5:
			if (ls)
				SET1_HARDWARE_OBJECT_PF5
			else 
				SET0_HARDWARE_OBJECT_PF5
		break;
		case HARDWARE_OBJECT_PF6:
			if (ls)
				SET1_HARDWARE_OBJECT_PF6
			else 
				SET0_HARDWARE_OBJECT_PF6
		break;
		case HARDWARE_OBJECT_PF7:
			if (ls)
				SET1_HARDWARE_OBJECT_PF7
			else 
				SET0_HARDWARE_OBJECT_PF7
		break;
	}
}


// init object
void init_led_object (char* cp) {

_O_LED_t*	p;
	p = (_O_LED_t*) cp;

	switch (p->hw_object_id) {

		case HARDWARE_OBJECT_PE0:
			INIT_HARDWARE_OBJECT_PE0
		break;
		case HARDWARE_OBJECT_PE1:
			INIT_HARDWARE_OBJECT_PE1
		break;
		case HARDWARE_OBJECT_PE2:
			INIT_HARDWARE_OBJECT_PE2
		break;
		case HARDWARE_OBJECT_PF4:
			INIT_HARDWARE_OBJECT_PF4
		break;
		case HARDWARE_OBJECT_PF5:
			INIT_HARDWARE_OBJECT_PF5
		break;
		case HARDWARE_OBJECT_PF6:
			INIT_HARDWARE_OBJECT_PF6
		break;
		case HARDWARE_OBJECT_PF7:
			INIT_HARDWARE_OBJECT_PF7
		break;
	}

}
