/** \file o_warning.c
 *  \brief Functions for the support of external buttons
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	The objects provided by this module are processed cyclicly triggered by a timer event. 
 *	They are not bound to any display page.
 *
 *	Implemented functions:
 *	- poll externally attached buttons
 *	- send EIB messages triggered by external buttons
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "o_button.h"
#include "System.h"
// counter to divide timer events down to 1sec intervals
uint8_t	secound_counter;

/** get_input_value (channel ID)
 *  Read the input value of hardware channel "channel ID" and return its value.
 */
uint8_t get_input_value (uint8_t id) {

	switch (id) {

		case HARDWARE_OBJECT_PE0:
			return CHECK_HARDWARE_INPUT_PE0;
		break;
		case HARDWARE_OBJECT_PE1:
			return CHECK_HARDWARE_INPUT_PE1;
		break;
		case HARDWARE_OBJECT_PE2:
			return CHECK_HARDWARE_INPUT_PE2;
		break;
		case HARDWARE_OBJECT_PF4:
			return CHECK_HARDWARE_INPUT_PF4;
		break;
		case HARDWARE_OBJECT_PF5:
			return CHECK_HARDWARE_INPUT_PF5;
		break;
		case HARDWARE_OBJECT_PF6:
			return CHECK_HARDWARE_INPUT_PF6;
		break;
		case HARDWARE_OBJECT_PF7:
			return CHECK_HARDWARE_INPUT_PF7;
		break;
	}

	// should never happen.
	return 0;
}

/** do_hardware_button ( EIB object ID, function code)
 *  Executes the button function "function code" for the EIB object "EIB object ID"
 */
void do_hardware_button (uint8_t obj, uint8_t fct) {

uint8_t eib_value;

	switch (fct & 0x07) {
		case HARDWARE_BUTTON_TOGGLE:
			// get current value
			eib_value = eib_get_object_8_value (obj);
			if (eib_value)
				eib_value = 0;
			else 
				eib_value = 1;
			// send value
			eib_G_DATA_request(get_group_address (obj), &eib_value, 0);
		break;
		case HARDWARE_BUTTON_SEND_0:
		case HARDWARE_BUTTON_REPEAT_SEND_0:
			eib_value = 0;
			eib_G_DATA_request(get_group_address (obj), &eib_value, 0);
		break;
		case HARDWARE_BUTTON_SEND_1:
		case HARDWARE_BUTTON_REPEAT_SEND_1:
			eib_value = 1;
			eib_G_DATA_request(get_group_address (obj), &eib_value, 0);
		break;
	}

}

/** do_hardware_button_repeat ( EIB object ID, function code)
 *  This function repeats the button function "function code" for the EIB object "EIB object ID"
 */
void do_hardware_button_repeat (uint8_t obj, uint8_t fct) {
uint8_t eib_value;

	switch (fct & 0x07) {
		case HARDWARE_BUTTON_REPEAT_SEND_0:
			eib_value = 0;
			eib_G_DATA_request(get_group_address (obj), &eib_value, 0);
		break;
		case HARDWARE_BUTTON_REPEAT_SEND_1:
			eib_value = 1;
			eib_G_DATA_request(get_group_address (obj), &eib_value, 0);
		break;
	}
}

/** Check_hardware_button (button object descriptor *)
 *  This function is called every 30ms and polls the external button state.
 *  
 */
void check_hardware_button (char* cp) {

_O_BUTTON_t*	p;
	p = (_O_BUTTON_t*) cp;

	// filter
	p->filter = p->filter >> 1;

	if (get_input_value (p->hw_object_id)) {
		p->filter |= 0x80;
	}

	if ((p->filter == 0x00) && (p->state & 0x80)) {
		// edge high->low
		p->state = 0x00; // set state and clear counter
		hwmon_show_button_event (p->hw_object_id, 0);
		// check, if edge should trigger action
		do_hardware_button (p->eib_object_send, p->function);
	}

	if ((p->filter == 0xff) && !(p->state & 0x80)) {
		// edge low->high
		p->state = 0x80; // set state and clear counter
		hwmon_show_button_event (p->hw_object_id, 1);
		// check, if edge should trigger action
		do_hardware_button (p->eib_object_send, p->function >> 3);
	}

	// check secound counter
	if (++secound_counter > 33) {

		secound_counter = 0;

		XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
		// check, if cyclic message should be sent
		if ((p->state & 0x7f) < 0x7f)
			p->state++;
		if ((p->state & 0x7f) == p->interval) {

			p->state &= 0x80;
		
			if (p->state & 0x80) {
				// repeat function for high level
				do_hardware_button_repeat (p->eib_object_send, p->function >> 3);
			}
			else {
				// repeat function for low level
				do_hardware_button_repeat (p->eib_object_send, p->function);
			} 
		}
	}
}

/** init_button_object (button object descriptor *)
 *  Initializes the external hardware input function.
 */
void init_button_object (char* cp) {

_O_BUTTON_t*	p;
	p = (_O_BUTTON_t*) cp;

	switch (p->hw_object_id) {

		case HARDWARE_OBJECT_PE0:
			INIT_HARDWARE_INPUT_PE0
		break;
		case HARDWARE_OBJECT_PE1:
			INIT_HARDWARE_INPUT_PE1
		break;
		case HARDWARE_OBJECT_PE2:
			INIT_HARDWARE_INPUT_PE2
		break;
		case HARDWARE_OBJECT_PF4:
			INIT_HARDWARE_INPUT_PF4
		break;
		case HARDWARE_OBJECT_PF5:
			INIT_HARDWARE_INPUT_PF5
		break;
		case HARDWARE_OBJECT_PF6:
			INIT_HARDWARE_INPUT_PF6
		break;
		case HARDWARE_OBJECT_PF7:
			INIT_HARDWARE_INPUT_PF7
		break;
	}

	// set current pin state information
	if (get_input_value (p->hw_object_id)) {
		p->filter = 0xff;
		p->state = 0x80;
	}
	else {
		p->filter = 0x00;
		p->state = 0x00;
	}

	secound_counter = 0;
}
