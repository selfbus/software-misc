/** \file EIBObjects.c
 *  \brief Functions for EIB Object handling
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- EIB object value treatment
 *		max. object length is 4 bytes
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "EIBObjects.h"
#include "math.h"

// clear all eib objject values
void eib_object_init () {

	// set object value bank
	XRAM_SELECT_BLOCK(XRAM_OBJECT_VALUE_PAGE);
	// clear all object values
	memset((void*) XRAM_BASE_ADDRESS, 0x00, EIB_OBJECT_DATA_SIZE * get_address_tab_length());

}


// update object values
// 0: no object updated
uint8_t eib_objects_process_msg (uint16_t address, uint8_t *data, uint8_t len, uint8_t apci) {

int object;
_EIB_OBJECT_DATA_t*	p;

	// get object #
	object = get_group_adress_index (address);
	// check object #
	if ((object < 0) || (object >= get_address_tab_length()))
		return 0;
	if (!( (apci == APCI_VALUE_RESPONSE) || (apci == APCI_VALUE_WRITE) ))
		return 0;

	// copy new data to object
	XRAM_SELECT_BLOCK(XRAM_OBJECT_VALUE_PAGE);
	p = (_EIB_OBJECT_DATA_t*) XRAM_BASE_ADDRESS;
	p += object;
	p->d0 = *data++;
	
	// FIXME: evaluate length
	p->d1 = *data++;
	p->d2 = *data++;
	p->d3 = *data;

	return 1;
}



// returns value of 8 bit objects
uint8_t eib_get_object_8_value (uint8_t object) {

_EIB_OBJECT_DATA_t*	p;

	p = (_EIB_OBJECT_DATA_t*) XRAM_BASE_ADDRESS;
	p += object;

	// set object value bank
	XRAM_SELECT_BLOCK(XRAM_OBJECT_VALUE_PAGE);

	return p->d0;

}

// returns value of 2 byte objects
uint16_t eib_get_object_16_value (uint8_t object) {

_EIB_OBJECT_DATA_t*	p;
uint16_t	result;

	p = (_EIB_OBJECT_DATA_t*) XRAM_BASE_ADDRESS;
	p += object;

	// set object value bank
	XRAM_SELECT_BLOCK(XRAM_OBJECT_VALUE_PAGE);

	result = p->d0;
	result = p->d1 | (result << 8);

	return result;

}

// returns value of 32 byte objects
uint32_t eib_get_object_32_value (uint8_t object) {

_EIB_OBJECT_DATA_t*	p;
uint32_t	result;

	p = (_EIB_OBJECT_DATA_t*) XRAM_BASE_ADDRESS;
	p += object;

	// set object value bank
	XRAM_SELECT_BLOCK(XRAM_OBJECT_VALUE_PAGE);

	result = p->d0;
	result = p->d1 | (result << 8);
	result = p->d2 | (result << 8);
	result = p->d3 | (result << 8);

	return result;

}

float eib_get_object_EIS5_value (uint8_t object) {

uint16_t val;
float fval;
int16_t	x;
uint8_t	exp;

	val = eib_get_object_16_value (object);
	x = val & 0x7ff;
	if (val & 0x8000) {
		x |= 0xf800;
	}
	exp = (val >> 11) & 0x0f;
	fval = ldexp (x, exp) / 100;			

	return fval;
}

// sends value of EIS5 float objects
void eib_set_object_EIS5_value (uint16_t address, float fval) {
										
uint8_t	eib_value[2];
uint8_t	exp;
int16_t ival;

	// process sign
	if (fval >= 0)
		eib_value [0] = 0x00;
	else eib_value [0] = 0x80;

	exp = 0;
	while ((fval > MAX_EIS5_MANTISSA) || (fval < MIN_EIS5_MANTISSA)) {
		exp++;
		fval /= 2;
	}
	eib_value[0] |= (exp << 3) & 0x78;
	ival = round (100*fval);
	eib_value[1] = ival & 0xff;
	eib_value[0] |= (ival >> 8) & 0x07;

	// send value to EIB object
	eib_G_DATA_request (address, eib_value, 2);
}

