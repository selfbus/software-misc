/** \file e_jumper.c
 *  \brief Functions for Jumper element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display Jumper element on page
 *	- branch to a page
 *	- time triggered page change (auto-jump)
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "e_jumper.h"


void draw_jumper_element (char* cp) {

_E_JUMPER_t*	p;

	p = (_E_JUMPER_t*) cp;

	draw_picture (p->picture_index_up, p->x_pos, p->y_pos);

}

uint8_t is_jumper_hit (char* cp, t_touch_event* evt) {

_E_JUMPER_t*	p;
uint16_t	width, height;

	p = (_E_JUMPER_t*) cp;

	get_picture_size (p->picture_index_up, &width, &height);

	if ((p->x_pos < evt->lx) && (p->y_pos < evt->ly) && (p->x_pos + width > evt->lx) && (p->y_pos + height > evt->ly)) {
		// jumper is touched
		return 1;
	}
	return 0;

} 

//  touch_state:
// 0: not selected
// 1: selected, but currently released
// 2: selected and pressed

// returns 0 if still active, returns 1 to release focus
uint8_t touch_jumper_element (char* cp, t_touch_event* evt, uint8_t *touch_state) {

_E_JUMPER_t*	p;
uint16_t	width, height;
uint8_t 	hit, hit_with_margin;

	p = (_E_JUMPER_t*) cp;
	get_picture_size (p->picture_index_down, &width, &height);
	hit = (p->x_pos < evt->lx) && (p->y_pos < evt->ly) && (p->x_pos + width > evt->lx) && (p->y_pos + height > evt->ly);
	hit_with_margin = (p->x_pos < evt->lx + TOUCH_MARGIN) && (p->y_pos < evt->ly + TOUCH_MARGIN) && 
					  (p->x_pos + width + TOUCH_MARGIN > evt->lx) && (p->y_pos + height + TOUCH_MARGIN > evt->ly);
	// are we already touched?
	if (*touch_state) {
		// yes, this element is still touched
		if (evt->state == RELEASED) {

			// user released the touch screen
			sound_play_clip (p->sound_index_up, 0);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

			if (hit_with_margin) {
				// execute activity
				if (p->target == SYSTEM_PAGE_SETUP) {
					create_system_info_screen ();
				}
				else if (p->target == SYSTEM_PAGE_LOCK) {
					create_screen_lock ();
				}
				else 
					set_page (p->target - SYSTEM_PAGES);
			}
			return 1;
		}
		else {
			// moved finger. Check, if element is still hit or not and adjust bitmap accordingly
			if (hit_with_margin && (*touch_state == 1)) {
				draw_picture (p->picture_index_down, p->x_pos, p->y_pos);
				*touch_state = 2;
			}
			if (!hit_with_margin && (*touch_state == 2)) {
				draw_picture (p->picture_index_up, p->x_pos, p->y_pos);
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
		draw_picture (p->picture_index_down, p->x_pos, p->y_pos);
		return 0;
	}
}

uint8_t auto_check_jumper_element (char* cp, uint8_t auto_jump_counter) {

_E_JUMPER_t*	p;

	p = (_E_JUMPER_t*) cp;
	if (p->target < SYSTEM_PAGES)
		return 0;
	if (auto_jump_counter && (p->auto_jump_delay == auto_jump_counter)) {
		set_page (p->target - SYSTEM_PAGES);
		return 1;
	}
	return 0;
}
