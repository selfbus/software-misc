/** \file e_button.c
 *  \brief Functions for LED touch buttons
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- send EIB messages triggered by touch buttons
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "e_button.h"

void draw_button_element (char* cp) {

_E_BUTTON_t*	p;

	p = (_E_BUTTON_t*) cp;

	draw_picture (p->picture_index_up, p->x_pos, p->y_pos);

}


uint8_t touch_button_element (char *cp, t_touch_event *evt, uint8_t *touch_state) {

_E_BUTTON_t*	p;
uint16_t	width, height;
uint8_t 	hit;
uint8_t		eib_object;
uint8_t		eib_value[2];
int16_t		new_value;
float 		fval;

	p = (_E_BUTTON_t*) cp;
	get_picture_size (p->picture_index_down, &width, &height);
	hit = (p->x_pos < evt->lx) && (p->y_pos < evt->ly) && (p->x_pos + width > evt->lx) && (p->y_pos + height > evt->ly);

	// are we already touched?
	if (*touch_state) {
		// yes, this element is still touched
		if (evt->state == RELEASED) {

			// user released the touch screen
			sound_play_clip (p->sound_index_up, 0);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

			// check, if we have to execute an activity
			switch (p->eib_function) {

				case EIB_BUTTON_FUNCTION_BRIGHTER:
				case EIB_BUTTON_FUNCTION_DARKER:
					// dimm stop
					eib_value[0] = 0x00;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
			}

			// set page descriptions bank for safety
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			draw_picture (p->picture_index_up, p->x_pos, p->y_pos);
			return 1;
		}
		else if (evt->state == TOUCHED_SHORT) {
			// display has been touched for short time and is released now

			switch (p->eib_function) {

				case EIB_BUTTON_FUNCTION_ON_BRIGHTER:
					// switch on
					eib_value[0] = 0x01;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_OFF_DARKER:
					// switch off
					eib_value[0] = 0x00;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_UP_STEPUP:
					// go up
					eib_value[0] = 0x00;
					eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_DOWN_STEPDOWN:
					// go down
					eib_value[0] = 0x01;
					eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_DELTA_EIS6:
					// add delta value to 8bit object and send it
					eib_object = p->eib_object0;
					eib_value[0] = eib_get_object_8_value (p->eib_object0);
					XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
					new_value = eib_value[0] + (int8_t) p->value[0];
					if (new_value < p->min) { 
						new_value = p->min;
					}
					if (new_value > p->max) { 
						new_value = p->max;
					}
					eib_value[0] = new_value & 0xff;
					eib_G_DATA_request(get_group_address(eib_object), eib_value, 1);
				break;
				case EIB_BUTTON_FUNCTION_DELTA_EIS5:
					// add delta value to 16bit EIS5 object and send it
					eib_object = p->eib_object0;
					fval = eib_get_object_EIS5_value (eib_object);
					XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
					fval += (float) ((int8_t) p->value[0])/10;
					// check, if the new value is inside of the bounds
					if (10*fval < p->min) { 
						fval = p->min;
						fval /= 10;
					}
					if (10*fval > p->max) { 
						fval = p->max;
						fval /= 10;
					}
					// send new value as EIS5
					eib_set_object_EIS5_value(get_group_address(eib_object), fval);
				break;
			}
		}
		else if (evt->state == TOUCHED_LONG) {

			switch (p->eib_function) {
				case EIB_BUTTON_FUNCTION_ON_BRIGHTER:
					// dimm up
                    new_value = (p->value[0] & 0x07); // Read from Project
                    if (new_value) {
                        eib_value[0] = (new_value | 0x08);  // Heller
                    }
                    else {
                        eib_value[0] = 0x09;
                    }                        
					eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_OFF_DARKER:
					// dimm down
                    new_value = (p->value[0] & 0x07); // Read from Project
                    if (new_value) {
                        eib_value[0] = new_value;
                    }
                    else {
                        eib_value[0] = 0x01;
                    }                        
					eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_UP_STEPUP:
					// go up
					eib_value[0] = 0x00;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_DOWN_STEPDOWN:
					// go down
					eib_value[0] = 0x01;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
			}
		}
		else if (evt->state == RELEASED_LONG) {
			// check, if we have to execute an activity
			switch (p->eib_function) {

				case EIB_BUTTON_FUNCTION_ON_BRIGHTER:
				case EIB_BUTTON_FUNCTION_OFF_DARKER:
					// dimm stop
					eib_value[0] = 0x00;
					eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
				break;
			}
		}
		else if (evt->state == TOUCHED_AUTO) {

			switch (p->eib_function) {
				case EIB_BUTTON_FUNCTION_STEPUP:
					// go up
					eib_value[0] = 0x00;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
				case EIB_BUTTON_FUNCTION_STEPDOWN:
					// go down
					eib_value[0] = 0x01;
					eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
				break;
			}
		}
		else if (evt->state == TOUCHED_AUTO_FAST) {

			switch (p->eib_function) {
				case EIB_BUTTON_FUNCTION_DELTA_EIS6:
					// add delta value to 8bit object and send it
					eib_object = p->eib_object0;
					eib_value[0] = eib_get_object_8_value (p->eib_object0);
					XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
					new_value = eib_value[0] + (int8_t) p->value[1];
					if (new_value < p->min) { 
						new_value = p->min;
					}
					if (new_value > p->max) { 
						new_value = p->max;
					}
					eib_value[0] = new_value & 0xff;
					eib_G_DATA_request(get_group_address(eib_object), eib_value, 1);
				break;
				case EIB_BUTTON_FUNCTION_DELTA_EIS5:
					// add delta value to 16bit EIS5 object and send it
					eib_object = p->eib_object0;
					fval = eib_get_object_EIS5_value (eib_object);
					XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
					fval += (float) ((int8_t) p->value[1])/10;
					// check, if the new value is inside of the bounds
					if (10*fval < p->min) { 
						fval = p->min;
						fval /= 10;
					}
					if (10*fval > p->max) { 
						fval = p->max;
						fval /= 10;
					}
					// send new value as EIS5
					eib_set_object_EIS5_value(get_group_address(eib_object), fval);
				break;
			}
		}
		else {
			// moved finger. Check, if element is still hit or not and adjust bitmap accordingly
			if (hit && (*touch_state == 1)) {
				draw_picture (p->picture_index_down, p->x_pos, p->y_pos);
				*touch_state = 2;
			}
			if (!hit && (*touch_state == 2)) {
				draw_picture (p->picture_index_up, p->x_pos, p->y_pos);
				*touch_state = 1;
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
		draw_picture (p->picture_index_down, p->x_pos, p->y_pos);
		// check, if we have to send a message
		// check, if we have to execute an activity
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		switch (p->eib_function) {
			case EIB_BUTTON_FUNCTION_TOGGLE:
				eib_object = p->eib_object0;
				eib_value[0] = eib_get_object_8_value (p->eib_object1);
				if (eib_value[0])
					eib_value[0] = 0x00;
				else
					eib_value[0] = 0x01;
				eib_G_DATA_request(get_group_address (eib_object), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_ON:
				// switch on
				eib_value[0] = 0x01;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_OFF:
				// switch off
				eib_value[0] = 0x00;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_BRIGHTER:
				// dimm up
                new_value = (p->value[0] & 0x07); // Read from Project
                if (new_value) {
                    eib_value[0] = (new_value | 0x08);  // Heller
                }
                else {
                	eib_value[0] = 0x09;
                }                    
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_DARKER:
				// dimm down
                new_value = (p->value[0] & 0x07); // Read from Project
                if (new_value) {
                    eib_value[0] = new_value;
                }
                else {
                    eib_value[0] = 0x01;
                }                
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_UP:
			case EIB_BUTTON_FUNCTION_STEPUP:
				// go up
				eib_value[0] = 0x00;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_DOWN:
			case EIB_BUTTON_FUNCTION_STEPDOWN:
				// go down
				eib_value[0] = 0x01;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_8BIT_VALUE:
				eib_object = p->eib_object0;
				eib_value[0] = p->value[0];
				eib_G_DATA_request(get_group_address (eib_object), eib_value, 1);
			break;
			case EIB_BUTTON_FUNCTION_16BIT_VALUE:
				eib_object = p->eib_object0;
				eib_value[0] = p->value[1];
				eib_value[1] = p->value[0];
				eib_G_DATA_request(get_group_address (eib_object), eib_value, 2);
			break;
		}
	}
	return 0; // we still aquire the focus
}

