/** \file ds1820.h
 *  \brief Constants and definitions for the communication with DS18x20 devices
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _DS1820_H_
#define _DS1820_H_

#include "System.h"
// unit 30ms
#define DS1820_CONVERSION_TICKS		27

#define DS1820_CMD_SKIP_ROM		0xCC
#define DS1820_CMD_CONVERT		0x44
#define DS1820_CMD_READ			0xBE

#define DS1820_FORMAT_EIS5		0x00

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		hw_channel;
uint8_t		eib_object;
uint8_t		repeat_frequency;
uint8_t		eis_number_format;
int8_t		temp_offset;
// Parameter: d5, d4: Flash frequency, d3, d2: state@1, d1, d0: state@0
uint8_t		parameter;
} _O_DS1820_t;

// public functions
void init_ds1820 (char*);
void terminate_ds1820 (char*);
// this is the cyclic tick for the DS18S20 state machine
void ds18S20_statemachine (char *cp);
// this is the cyclic tick for the DS18B20 state machine
void ds18B20_statemachine (char *cp);


#endif //_DS1820_H_
