/** \file cyclic.h
 *  \brief Constants and definitions for the control of cyclicly called objects
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *  Copyright (c) 2013 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _CYCLIC_H_
#define _CYCLIC_H_

#include "System.h"
#include "MemoryMap.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_count;
uint8_t		checksum;
} _CYCLIC_DESCRIPTOR_t;

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
} _CYCLIC_ELEMENT_t;

#define	CYCLIC_ELEMENT_TYPE_BUTTON		0
#define	CYCLIC_ELEMENT_TYPE_DS18S20		1
#define	CYCLIC_ELEMENT_TYPE_IR			2
#define	CYCLIC_ELEMENT_TYPE_DS18B20		3
#define	CYCLIC_ELEMENT_TYPE_DHT11		4
// moves the page descriptions from Flash into RAM
// returns 0 if ok
// returns 1 on checksum error
uint8_t move_cyclic_descriptions (uint32_t, uint32_t);

// check objects on event
void lcd_cyclic_process_event (void);

// init hardware
void lcd_init_cyclic_objects (void);


#endif // _CYCLIC_H_
