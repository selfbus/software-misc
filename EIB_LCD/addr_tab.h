/** \file addr_tab.h
 *  \brief Constants and definitions for the EIB group address handling
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _GROUP_H_
#define _GROUP_H_

#include "System.h"
#include "MemoryMap.h"
#include "tft_io.h"

// moves the address table from Flash into RAM
// returns 0 if ok
// returns 1 on checksum error
uint8_t move_address_table (uint32_t, uint32_t);

// checks, if address exists in sorted table and returns the index.
// -1: not existing
// 0: first address
// 1...
int get_group_adress_index (uint16_t);

// get length of address table
uint16_t get_address_tab_length (void);
// get address i
uint16_t get_group_address (uint8_t); 

#endif // _GROUP_H_
