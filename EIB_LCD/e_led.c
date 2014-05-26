/** \file e_led.c
 *  \brief Functions for LED element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display LED element on page
 *	- update LED element on reception of new EIB messages
 *	- update LED element based on timer trigger
 *	- send EIB message triggered by touch event
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "e_led.h"
#include "System.h"


void draw_led_element (char* cp) {

_E_LED_t*	p;
uint8_t		m;
uint8_t		s;

	p = (_E_LED_t*) cp;

	XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

	/* binary or radio button function? */
	if (p->parameter & LED_PARAMETER_RADIO) {
		s = p->repeat_radio_value == eib_get_object_8_value (p->eib_object_listen);
	}
	else {
		m = 1 << (p->parameter & LED_PARAMETER_BITPOS);
		s = eib_get_object_8_value (p->eib_object_listen) & m;
	}
	XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
	if (s) {
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		if (p->parameter & LED_PARAMETER_WARNING) {
			draw_picture (p->picture_warning_index, p->x_pos, p->y_pos);
			set_backlight_on ();
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sound_play_clip (p->sound_index_warning, p->repeat_radio_value);
		}
		else {
			draw_picture (p->picture_on_index, p->x_pos, p->y_pos);
		}
	}
	else {
		draw_picture (p->picture_off_index, p->x_pos, p->y_pos);
	}
}


void check_led_element (char* cp, int obj) {

_E_LED_t*	p;

	p = (_E_LED_t*) cp;

	if (p->eib_object_listen == obj)
		draw_led_element (cp);
}

uint8_t touch_led_element (char *cp, t_touch_event *evt, uint8_t *touch_state) {

_E_LED_t*	p;
uint16_t	width, height;
uint8_t 	hit;
uint8_t		eib_value;

	p = (_E_LED_t*) cp;
	// check, if this element is sensitive to touch events
	if (!(p->parameter & LED_PARAMETER_SEND))
		return 1;

	get_picture_size (p->picture_on_index, &width, &height);
	hit = (p->x_pos < evt->lx) && (p->y_pos < evt->ly) && (p->x_pos + width > evt->lx) && (p->y_pos + height > evt->ly);

	// are we already touched?
	if (*touch_state) {
		// yes, this element is still touched
		if (evt->state == RELEASED) {

			// user released the touch screen
			sound_play_clip (p->sound_index_up, 0);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

			return 1;
		}
		else {
			// moved finger. Check, if element is still hit or not and adjust bitmap accordingly
			if (hit && (*touch_state == 1)) {
				*touch_state = 2;
			}
			if (!hit && (*touch_state == 2)) {
				*touch_state = 1;
			}
			return 0;
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

		/* execute activity depending on the LED function type */
		if (p->parameter & LED_PARAMETER_WARNING) {
			/* Warning element can switch off only */
			eib_value = 0x00;
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			eib_G_DATA_request(get_group_address (p->eib_object_send), &eib_value, 0);
		}
		else if (p->parameter & LED_PARAMETER_RADIO) {
			/* Radio button element always sends its own ID */
			eib_value = p->repeat_radio_value;
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			eib_G_DATA_request(get_group_address (p->eib_object_send), &eib_value, 1);
		}
		else {
			/* Indicator LED always toggles its object value */
			eib_value = eib_get_object_8_value (p->eib_object_listen);
			if (eib_value)
				eib_value = 0x00;
			else
				eib_value = 0x01;
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			eib_G_DATA_request(get_group_address (p->eib_object_send), &eib_value, 0);
		}

		return 0;
	}
}

uint8_t check_led_warning_state (char* cp, uint8_t warning_toggle) {

_E_LED_t*	p;

		p = (_E_LED_t*) cp;

		/* warning function? */
		if (p->parameter & LED_PARAMETER_WARNING) {
			if (eib_get_object_8_value (p->eib_object_listen)) {
				XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
				if (warning_toggle)
					draw_picture (p->picture_warning_index, p->x_pos, p->y_pos);
				else
					draw_picture (p->picture_on_index, p->x_pos, p->y_pos);
				return 1;
			}
		}
		return 0;
}
