/** \file EIBObjects.h
 *  \brief Constants and definitions for EIB object data handling
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _EIB_OBJECTS_H_
#define _EIB_OBJECTS_H_

#include "System.h"

/**
* @brief EIB objects description
*/
typedef struct __attribute__ ((packed)) {
uint8_t		d0;	// object data 0
uint8_t		d1;	// object data 1
uint8_t		d2;	// object data 2
uint8_t		d3;	// object data 3
} _EIB_OBJECT_DATA_t;

// EIB objects allocate 4 bytes data
#define EIB_OBJECT_DATA_SIZE	4

#define MAX_EIS5_MANTISSA 20.47
#define MIN_EIS5_MANTISSA -20.48

void eib_object_init (void);
uint8_t eib_get_object_8_value (uint8_t);
// returns value of 2 byte float objects
uint16_t eib_get_object_16_value (uint8_t);
// returns value of 4 byte float objects
uint32_t eib_get_object_32_value (uint8_t);
// returns value of EIS5 float objects
float eib_get_object_EIS5_value (uint8_t);

// sends value of EIS5 float objects
void eib_set_object_EIS5_value (uint16_t, float);
// handle EIB group message
uint8_t eib_objects_process_msg (uint16_t, uint8_t*, uint8_t, uint8_t);


#endif // _EIB_OBJECTS_H_
