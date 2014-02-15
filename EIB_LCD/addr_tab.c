/** \file addr_tab.c
 *  \brief Functions for EIB group address table control
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- move address table from Nand Flash into XRAM
 *	- get table index of received group address
 *	- get group address of table index
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "addr_tab.h"

uint16_t address_tab_length;

// moves address table from Flash into RAM. Purpose is fast and easy access to Bytes.
// flash offset: start address in Flash
// size: size of page descriptions in Byte
uint8_t move_address_table (uint32_t flash_offset, uint32_t size) {

	// we can only handle sizes up to one XRAM page
	if (size > XRAM_BANK_SIZE)
		return 2;

	// move page descriptions from Flash into XRAM
	copy_Flash_to_XRAM ((flash_offset >> 16) & 0xff, flash_offset & 0xffff, XRAM_GROUP_ADDR, size);
	address_tab_length = size >> 1;

	return 0;
}

// checks, if address exists in sorted table and returns the index.
// -1: not existing
// 0: first address
// 1...
int get_group_adress_index (uint16_t addr) {

uint16_t *po;	// pointer to address
uint8_t	 save_xram_page;
int i;

	save_xram_page = XRAM_GET_SELECTED_BLOCK;

	// set address descriptions bank
	XRAM_SELECT_BLOCK(XRAM_GROUP_PAGE);
	po = (uint16_t*) XRAM_BASE_ADDRESS;

	for (i = 0; i < address_tab_length; i++) {
		if (addr == *po) {
			XRAM_SELECT_BLOCK(save_xram_page);
			return i;
		}
		else {
			po++;
		}
	}

	XRAM_SELECT_BLOCK(save_xram_page);
	return -1;
}

// get address i
uint16_t get_group_address (uint8_t i) {

uint16_t *po;	// pointer to address

	// set address descriptions bank
	XRAM_SELECT_BLOCK(XRAM_GROUP_PAGE);
	po = (uint16_t*) XRAM_BASE_ADDRESS;
	po += i;
	return *po;
}

uint16_t get_address_tab_length (void) {

	return address_tab_length;

}


