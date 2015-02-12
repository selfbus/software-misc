/** \file listen.c
 *  \brief Functions for the always listening elements
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- move "always listen" objects from Nand Flash into XRAM
 *	- inform all "always listen" objects about a new EIB message
 *	- inform all "always listen" objects about a new cyclic timer event
 *
 *	Copyright (c) 2011-2015 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "listen.h"
#include "o_backlight.h"
#include "o_led.h"
#include "o_warning.h"
#include "o_timeout.h"

uint8_t listen_descriptions_validated;
volatile uint8_t listen_objects_timer;
volatile uint8_t listen_objects_timer_1s;
volatile uint8_t listen_objects_timer_flags;

// moves listening elements descriptions from Flash into RAM. Purpose is fast and easy access to Bytes.
// flash offset: start address in Flash
// size: size of page descriptions in Byte
uint8_t move_listen_descriptions (uint32_t flash_offset, uint32_t size) {

uint8_t checksum;
uint16_t i;


	// we can only handle sizes up to one XRAM page
	if (size > XRAM_BANK_SIZE)
		return 2;

	// move page descriptions from Flash into XRAM
	copy_Flash_to_XRAM ((flash_offset >> 16) & 0xff, flash_offset & 0xffff, XRAM_LISTEN_ELEMENTS_ADDR, size);

	checksum = 0;
	// validate checksum in XRAM
	for (i = 0; i < size; i++) {
		checksum ^= INB (XRAM_BASE_ADDRESS + i);
	}

	if (checksum)
		return 1;

	listen_descriptions_validated = 1;

	return 0;
}

/**
* @brief processes a new group message received from the EIB
*
*/
void lcd_listen_process_msg (uint16_t address) {

char* p;
_LISTEN_ELEMENT_t	*listen_element;
uint8_t	element_count;
int i;
int	eib_object;

	if (flash_content_bad)
		return;

	eib_object = get_group_adress_index (address);
	if (eib_object < 0)
		return;

	// poll all components of active page and check, if they match the eib address
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
	p = (char*)XRAM_BASE_ADDRESS;
	element_count = ((_LISTEN_DESCRIPTOR_t*) p)->element_count;
	p += sizeof (_LISTEN_DESCRIPTOR_t);

	// iterate all listening elements
	for (i = 0; i < element_count; i++) {
		listen_element = (_LISTEN_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

		switch (listen_element->element_type) {
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE:
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE:
				check_backlight_object (p, eib_object);
			break;
			case LISTEN_ELEMENT_TYPE_LED:
			break;
			case LISTEN_ELEMENT_TYPE_WARNING:
				check_warning_object (p, eib_object);
			break;
			case LISTEN_ELEMENT_TIMEOUT:
				check_timeout_object (p, eib_object);
			break;
#ifdef LCD_DEBUG
			default: printf_P (PSTR("%s():%d unknown listen element %d\n"), __FUNCTION__, __LINE__, listen_element->element_type);
#endif
		}

		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
		p += listen_element->element_size;
	}
}

/**
* @brief init all listen elements: setup hardware and set default values
*
*/
void lcd_init_listen_objects (void) {

char* p;
_LISTEN_ELEMENT_t	*listen_element;
uint8_t	element_count;
int i;

	if (flash_content_bad)
		return;

	// poll all listen components and check, if they need hardware setup
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
	p = (char*)XRAM_BASE_ADDRESS;
	element_count = ((_LISTEN_DESCRIPTOR_t*) p)->element_count;
	p += sizeof (_LISTEN_DESCRIPTOR_t);

	// iterate all listening elements
	for (i = 0; i < element_count; i++) {
		listen_element = (_LISTEN_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

		switch (listen_element->element_type) {

			case LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE:
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE:
				init_backlight_object (p);
			break;
			case LISTEN_ELEMENT_TYPE_LED:
				init_led_object (p);
			break;
            case LISTEN_ELEMENT_TYPE_WARNING:
            // nothing to do for TYPE_WARNING
            break;
			case LISTEN_ELEMENT_TIMEOUT:
				init_timeout_object (p);
			break;
#ifdef LCD_DEBUG
			default: printf_P (PSTR("%s():%d unknown listen element %d\n"), __FUNCTION__, __LINE__, listen_element->element_type);
#endif
		}

		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
		p += listen_element->element_size;
	}

	listen_objects_timer = 0;
	listen_objects_timer_1s = 0;
	listen_objects_timer_flags = 0;
}


// get timeout counter value
uint16_t lcd_get_timeout_counter (uint8_t eib_object) {

char* p;
_LISTEN_ELEMENT_t	*listen_element;
uint8_t	element_count;
int i;
uint16_t val = 0;

	// poll all components of active page and check, if they match the eib address
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
	p = (char*)XRAM_BASE_ADDRESS;
	element_count = ((_LISTEN_DESCRIPTOR_t*) p)->element_count;
	p += sizeof (_LISTEN_DESCRIPTOR_t);

	// iterate all listening elements
	for (i = 0; i < element_count; i++) {
		listen_element = (_LISTEN_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

		switch (listen_element->element_type) {
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE:
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE:
			case LISTEN_ELEMENT_TYPE_LED:
			break;
			case LISTEN_ELEMENT_TIMEOUT:
				if (get_timeout_object_counter (p, eib_object, &val))
					return val;
			break;
		}

		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
		p += listen_element->element_size;
	}
	// should never happen:
	return 0xffff;
}



/**
* @brief processes a new Timer event from main loop
*
*/
void lcd_listen_timer_event () {

char* p;
_LISTEN_ELEMENT_t	*listen_element;
uint8_t	element_count;
int i;

	if (flash_content_bad)
		return;

	if (++listen_objects_timer > LISTEN_OBJECTS_TIMER_MAX) {
		listen_objects_timer = 0;
		// calculate timer flags
		listen_objects_timer_flags++;
	}

	if (++listen_objects_timer_1s > LISTEN_OBJECTS_TIMER_1SEC) {
		listen_objects_timer_1s = 0;
	}

	// poll all components of active page and check, if they match the eib address
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
	p = (char*)XRAM_BASE_ADDRESS;
	element_count = ((_LISTEN_DESCRIPTOR_t*) p)->element_count;
	p += sizeof (_LISTEN_DESCRIPTOR_t);

	// iterate all listening elements
	for (i = 0; i < element_count; i++) {
		listen_element = (_LISTEN_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);

		switch (listen_element->element_type) {
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_IDLE:
			case LISTEN_ELEMENT_TYPE_BACKLIGHT_ACTIVE:
			break;
			case LISTEN_ELEMENT_TYPE_LED:
				check_led_object (p, listen_objects_timer_flags);
			break;
			case LISTEN_ELEMENT_TIMEOUT:
				if (!listen_objects_timer_1s)
					tick_timeout_object (p);
			break;
		}

		XRAM_SELECT_BLOCK(XRAM_LISTEN_ELEMENTS_PAGE);
		p += listen_element->element_size;
	}
}

