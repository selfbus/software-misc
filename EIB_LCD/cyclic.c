/** \file cyclic.c
 *  \brief Functions for cyclically called objects
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- move cyclically called objects description table from Nand Flash into XRAM
 *	- init all cyclically called objects at start-up
 *	- poll all cyclic objects triggered by timer event
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *  Copyright (c) 2013 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "cyclic.h"
#include "o_backlight.h"
#include "ir_button.h"
#include "ds1820.h"
#include "dht11.h"
#include "o_button.h"

uint8_t cyclic_descriptions_validated;

// moves cyclic elements descriptions from Flash into RAM. Purpose is fast and easy access to Bytes.
// flash offset: start address in Flash
// size: size of page descriptions in Byte
uint8_t move_cyclic_descriptions (uint32_t flash_offset, uint32_t size) {

uint8_t checksum;
uint16_t i;


	// we can only handle sizes up to one XRAM page
	if (size > XRAM_BANK_SIZE)
		return 2;

	// move page descriptions from Flash into XRAM
	copy_Flash_to_XRAM ((flash_offset >> 16) & 0xff, flash_offset & 0xffff, XRAM_CYCLIC_ELEMENTS_ADDR, size);

	checksum = 0;
	// validate checksum in XRAM
	for (i = 0; i < size; i++) {
		checksum ^= INB (XRAM_BASE_ADDRESS + i);
	}

	if (checksum)
		return 1;

	cyclic_descriptions_validated = 1;

	return 0;
}


uint8_t rc5_event; // RC5_PRESSED_NEW, RC5_RELEASED_SHORT (<3 msg), RC5_PRESSED_LONG (> 2msg), RC5_RELEASED_LONG
uint8_t rc5_last_address;
uint8_t rc5_last_command;
uint8_t rc5_counter;
uint8_t rc5_gap_timer;

/**
* @brief manages cyclically called objects
*
* currently called every 30ms
*/
void lcd_cyclic_process_event (void) {

char* p;
_CYCLIC_ELEMENT_t	*cyclic_element;
uint8_t	element_count;
int i;

	if ((flash_content_bad) || (!cyclic_descriptions_validated))
		return;

	// check RC5 reception
	rc5_event = 0;
	if (rc5_full) {
		if (rc5_decode_message() ) {
			if (rc5_counter++) {
				if (rc5_counter == 5)
					rc5_event = RC5_PRESSED_LONG;
				if (rc5_counter == 16) {
					rc5_counter = 6;
					rc5_event = RC5_PRESSED_AUTO;
				}
			}
			else {
				rc5_event = RC5_PRESSED_NEW;
			}
		}
		// time * 30ms between messages
		rc5_gap_timer = 8;
	}
	else {
		if (rc5_gap_timer) {
			if (!(--rc5_gap_timer)) {
				if (rc5_counter < 5)
					rc5_event = RC5_RELEASED_SHORT;
				else rc5_event = RC5_RELEASED_LONG;
				rc5_counter = 0;
			}
		}
	}

	if (rc5_event == RC5_PRESSED_NEW)
		hwmon_show_ir_event();
	// poll all cyclic components and check, if they need service
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
	p = (char*)XRAM_BASE_ADDRESS;
	element_count = ((_CYCLIC_DESCRIPTOR_t*) p)->element_count;
	p += sizeof (_CYCLIC_DESCRIPTOR_t);

	// iterate all cyclic elements
	for (i = 0; i < element_count; i++) {
		cyclic_element = (_CYCLIC_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);

		switch (cyclic_element->element_type) {
			case CYCLIC_ELEMENT_TYPE_BUTTON:
				check_hardware_button (p);
			break;
			case CYCLIC_ELEMENT_TYPE_DS18S20:
				ds18S20_statemachine (p);
			break;
			case CYCLIC_ELEMENT_TYPE_DS18B20:
				ds18B20_statemachine (p);
			break;
			case CYCLIC_ELEMENT_TYPE_DHT11:
				dht_statemachine (p);
			break;
			case CYCLIC_ELEMENT_TYPE_IR:
				if (rc5_event) {
					ir_button_pressed (p, rc5_event);
				}
			break;
#ifdef LCD_DEBUG
			default: printf_P (PSTR("unknown cyclic element %d\n"), cyclic_element->element_type);
#endif
		}

		XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
		p += cyclic_element->element_size;
	}
}


/**
* @brief init all listen elements: setup hardware and set default values
*
*/
void lcd_init_cyclic_objects (void) {

char* p;
_CYCLIC_ELEMENT_t	*cyclic_element;
uint8_t	element_count;
int i;

	if ((flash_content_bad) || (!cyclic_descriptions_validated))
		return;

	// init RC5 timers
	rc5_gap_timer = 0;
	rc5_counter = 0;

	// poll all cyclic components and check, if they need hardware setup
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
	p = (char*)XRAM_BASE_ADDRESS;
	element_count = ((_CYCLIC_DESCRIPTOR_t*) p)->element_count;
	p += sizeof (_CYCLIC_DESCRIPTOR_t);

	// iterate all listening elements
	for (i = 0; i < element_count; i++) {
		cyclic_element = (_CYCLIC_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);

		switch (cyclic_element->element_type) {

			case CYCLIC_ELEMENT_TYPE_BUTTON:
				init_button_object (p);
			break;
			case CYCLIC_ELEMENT_TYPE_DS18S20:
				init_ds1820 (p);
			break;
			case CYCLIC_ELEMENT_TYPE_DS18B20:
				init_ds1820 (p);
			break;
			case CYCLIC_ELEMENT_TYPE_DHT11:
				init_dht (p);
			break;
			case CYCLIC_ELEMENT_TYPE_IR:
				init_rc5 ();
			break;
#ifdef HW_DEBUG
			default: printf_P (PSTR("unknown cyclic element %d\n"), cyclic_element->element_type);
#endif
		}

		XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);
		p += cyclic_element->element_size;
	}
}

