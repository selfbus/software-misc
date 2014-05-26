/** \file e_sbutton.c
 *  \brief Functions for state button element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display picture representing the state button element:
 *   	- object is off and button is up
 *   	- object is off and button is down
 *   	- object is on and button is up
 *   	- object is on and button is down
 *	- send EIB message triggered by touch event
 *		button function is toggle only
 *	- change picture on reception of EIB messages
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "e_sbutton.h"


void draw_sbutton_element (char* cp, uint8_t touch_state) {

_E_SBUTTON_t*	p;
uint8_t 		eib_value;

	p = (_E_SBUTTON_t*) cp;

	XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

	eib_value = eib_get_object_8_value (p->eib_object_listen);
	if (touch_state == 2) {
		if (eib_value) {
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			draw_picture (p->picture_index_down_on, p->x_pos, p->y_pos);
		}
		else {
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			draw_picture (p->picture_index_down_off, p->x_pos, p->y_pos);
		}
	}
	else {
		if (eib_value) {
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			draw_picture (p->picture_index_up_on, p->x_pos, p->y_pos);
		}
		else {
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			draw_picture (p->picture_index_up_off, p->x_pos, p->y_pos);
		}
	}
}

void check_sbutton_element (char* cp, int obj, uint8_t is_active, uint8_t state) {

_E_SBUTTON_t*	p;

	p = (_E_SBUTTON_t*) cp;

	if (p->eib_object_listen == obj)
		draw_sbutton_element (cp, (is_active)? state : 0);
}


uint8_t touch_sbutton_element (char *cp, t_touch_event *evt, uint8_t *touch_state) {

_E_SBUTTON_t*	p;
uint16_t	width, height;
uint8_t 	hit;
uint8_t		eib_object;
uint8_t		eib_value;

	p = (_E_SBUTTON_t*) cp;
	get_picture_size (p->picture_index_down_off, &width, &height);
	hit = (p->x_pos < evt->lx) && (p->y_pos < evt->ly) && (p->x_pos + width > evt->lx) && (p->y_pos + height > evt->ly);

	// are we already touched?
	if (*touch_state) {
		// yes, this element is still touched
		if (evt->state == RELEASED) {

			// user released the touch screen
			sound_play_clip (p->sound_index_up, 0);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

			// set page descriptions bank for safety
			draw_sbutton_element (cp, 0);
			return 1;
		}
		else if (evt->state == MOVE) {

			// moved finger. Check, if element is still hit or not and adjust bitmap accordingly
			if (hit && (*touch_state == 1)) {
				*touch_state = 2;
				draw_sbutton_element (cp, 2);
			}
			if (!hit && (*touch_state == 2)) {
				*touch_state = 1;
				draw_sbutton_element (cp, 1);
			}
		}
	}
	else {
		// no, element is not yet touched
		if (!hit)
			return 1;
		// we are touched first time, if event is TOUCHED
		if (evt->state == TOUCHED) {
			*touch_state = 2;
			sound_play_clip (p->sound_index_down, 0);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		}
		else return 1;
		// now we are touched first time
		draw_sbutton_element (cp, 2);
		// check, if we have to send a message
		// check, if we have to execute an activity
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		switch (p->eib_function) {
			case EIB_BUTTON_FUNCTION_TOGGLE:
				eib_object = p->eib_object_send;
				eib_value = eib_get_object_8_value (p->eib_object_listen);
				if (eib_value)
					eib_value = 0x00;
				else
					eib_value = 0x01;
				eib_G_DATA_request(get_group_address (eib_object), &eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_ON:
				// switch on
				eib_value = 0x01;
				eib_G_DATA_request(get_group_address (p->eib_object_send), &eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_OFF:
				// switch off
				eib_value = 0x00;
				eib_G_DATA_request(get_group_address (p->eib_object_send), &eib_value, 0);
			break;
		}
	}
	return 0; // we still aquire the focus
}

