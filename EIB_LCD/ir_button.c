/** \file ir_button.c
 *  \brief Functions triggered by IR buttons
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- send EIB messages triggered by IR buttons
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "ir_button.h"

void ir_button_pressed (char *cp, uint8_t evt) {

_IR_BUTTON_t*	p;
uint8_t		eib_object;
uint8_t		eib_value[2];
int			new_value;
float		fval;

	p = (_IR_BUTTON_t*) cp;
	
	// check, if address and command match
	if ((p->rc5_address != rc5_a) || (p->rc5_command != rc5_c))
		return;

	if (evt == RC5_RELEASED_LONG) {

		// check, if we have to execute an activity
		switch (p->eib_function) {

			case EIB_BUTTON_FUNCTION_BRIGHTER:
			case EIB_BUTTON_FUNCTION_DARKER:
				// dimm stop
				eib_value[0] = 0x00;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_ON_BRIGHTER:
			case EIB_BUTTON_FUNCTION_OFF_DARKER:
				// dimm stop
				eib_value[0] = 0x00;
				eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
			break;
		}
	}
	else if (evt == RC5_RELEASED_SHORT) {
		// button has been touched for short time and is released now

		switch (p->eib_function) {

			case EIB_BUTTON_FUNCTION_BRIGHTER:
			case EIB_BUTTON_FUNCTION_DARKER:
				// dimm stop
				eib_value[0] = 0x00;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
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
		}
	}
	else if (evt == RC5_PRESSED_LONG) {

		switch (p->eib_function) {
			case EIB_BUTTON_FUNCTION_ON_BRIGHTER:
				// dimm up
				eib_value[0] = 0x09;
				eib_G_DATA_request(get_group_address (p->eib_object1), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_OFF_DARKER:
				// dimm down
				eib_value[0] = 0x01;
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
	else if (evt == RC5_PRESSED_AUTO) {

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
			case EIB_BUTTON_FUNCTION_DELTA_EIS6:
				// add delta value to 8bit object and send it
				eib_object = p->eib_object0;
				eib_value[0] = eib_get_object_8_value (p->eib_object0);
				XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
				new_value = eib_value[0] + (int8_t) p->value[0];
/*				if (new_value < p->min) { 
					new_value = p->min;
				}
				if (new_value > p->max) { 
					new_value = p->max;
				} */
				if (new_value < 0) { 
					new_value = 0;
				}
				if (new_value > 255) { 
					new_value = 255;
				}
				eib_value[0] = new_value & 0xff;
				eib_G_DATA_request(get_group_address(eib_object), eib_value, 1);
			break;
			case EIB_BUTTON_FUNCTION_DELTA_EIS5:
				// add delta value to 16bit EIS5 object and send it
				eib_object = p->eib_object0;
				fval = eib_get_object_EIS5_value (eib_object);
				XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
				fval += (float) ((int8_t) p->value[0])/10;
				// check, if the new value is inside of the bounds
/*				if (10*fval < p->min) { 
					fval = p->min;
					fval /= 10;
				}
				if (10*fval > p->max) { 
					fval = p->max;
					fval /= 10;
				} */
				if (fval < -100.0) { 
					fval = -100.0;
				}
				if (fval > 100.0) { 
					fval = 100.0;
				}
				// send new value as EIS5
				eib_set_object_EIS5_value(get_group_address(eib_object), fval);
			break;
		}
	}
	else if (evt == RC5_PRESSED_NEW) {
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
				eib_value[0] = 0x09;
				eib_G_DATA_request(get_group_address (p->eib_object0), eib_value, 0);
			break;
			case EIB_BUTTON_FUNCTION_DARKER:
				// dimm down
				eib_value[0] = 0x01;
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
			case EIB_BUTTON_FUNCTION_DELTA_EIS6:
				// add delta value to 8bit object and send it
				eib_object = p->eib_object0;
				eib_value[0] = eib_get_object_8_value (p->eib_object0);
				XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
				new_value = eib_value[0] + (int8_t) p->value[0];
/*				if (new_value < p->min) { 
					new_value = p->min;
				}
				if (new_value > p->max) { 
					new_value = p->max;
				} */
				if (new_value < 0) { 
					new_value = 0;
				}
				if (new_value > 255) { 
					new_value = 255;
				}
				eib_value[0] = new_value & 0xff;
				eib_G_DATA_request(get_group_address(eib_object), eib_value, 1);
			break;
			case EIB_BUTTON_FUNCTION_DELTA_EIS5:
				// add delta value to 16bit EIS5 object and send it
				eib_object = p->eib_object0;
				fval = eib_get_object_EIS5_value (eib_object);
				XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
				fval += (float) ((int8_t) p->value[0])/10;
				// check, if the new value is inside of the bounds
/*				if (10*fval < p->min) { 
					fval = p->min;
					fval /= 10;
				}
				if (10*fval > p->max) { 
					fval = p->max;
					fval /= 10;
				} */
				if (fval < -100.0) { 
					fval = -100.0;
				}
				if (fval > 100.0) { 
					fval = 100.0;
				}
				// send new value as EIS5
				eib_set_object_EIS5_value(get_group_address(eib_object), fval);
			break;
		}
	}
}

