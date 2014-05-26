/** \file o_warning.h
 *  \brief Constants for the event support
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	- object definition for automatic page change
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */

#ifndef _O_WARNING_H_
#define _O_WARNING_H_

#include "System.h"

typedef struct __attribute__ ((packed)) {
uint8_t		element_size;
uint8_t		element_type;
uint8_t		warning_object_id;
uint8_t		destination_page;
} _O_WARNING_t;

// check for update from EIB
void check_warning_object (char*, uint8_t);


#endif // _O_WARNING_H_
