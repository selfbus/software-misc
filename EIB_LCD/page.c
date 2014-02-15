/** \file page.c
 *  \brief Functions for the page control functions for the display
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display pages
 *	- update page contents at reception of EIB messages
 *	- cyclicly update page contents on timer trigger
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "page.h"

uint8_t page_descriptions_validated;
uint8_t	active_page;

char*	active_element; // this element has been "touched"
uint8_t	active_element_state;
uint8_t	(*touch_function)(char*, t_touch_event*, uint8_t*);	// this function handles the touch event for the active component
uint8_t auto_jump_counter;
uint8_t	t_divider;
uint8_t	warning_state; // 0x81 = show warning picture & sound, 1 = show picture WARNING, 0 = show picture on

// moves page descriptions from Flash into RAM. Purpose is fast and easy access to Bytes.
// flash offset: start address in Flash
// size: size of page descriptions in Byte
uint8_t move_page_descriptions (uint32_t flash_offset, uint32_t size) {

uint8_t checksum;
uint16_t i;

	// we can only handle sizes up to one XRAM page
	if (size > XRAM_BANK_SIZE)
		return 2;

	// move page descriptions from Flash into XRAM
	copy_Flash_to_XRAM ((flash_offset >> 16) & 0xff, flash_offset & 0xffff, XRAM_PAGE_ADDR, size);

	checksum = 0;
	// validate checksum in XRAM
	for (i = 0; i < size; i++) {
		checksum ^= INB (XRAM_BASE_ADDRESS + i);
	}

	if (checksum)
		return 1;

	page_descriptions_validated = 1;
	active_page = 0;
	
	// init counter for divider to secounds
	t_divider = 0;

	return 0;
}

// returns an pointer to the page offset
// page = 0,1,...
char* get_page_descriptor (uint8_t page) {

uint16_t page_offset, *po;
 
	// set page descriptions bank
	XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
	// check, if page is valid
	if (page >= INB (XRAM_BASE_ADDRESS))
		page = 0;
	// page is valid. Address offset is stored at table offset 2+2*page
	po = (uint16_t*) XRAM_BASE_ADDRESS;
	po += (page+1); // skip header 
	page_offset = *po; // get offset of page
	page_offset += 2*(1+INB (XRAM_BASE_ADDRESS)); // skip header and page offset table
	return (char*) page_offset + XRAM_BASE_ADDRESS;
}

// fill screen with monochrom color
void fill_screen (char* cp) {

_E_BACKGROUND_t*	p;

	p = (_E_BACKGROUND_t*) cp;
	tft_pant( BYTE2COLOR (p->red, p->green, p->blue) );
}


// set page active and redraw screen contents
void set_page (uint8_t page){

char* p;
_PAGE_DESCRIPTOR_t	*page_table;
_PAGE_ELEMENT_t		*page_element;
uint8_t	element_count;
int i;

	if (!flash_content_ok)
		return;
	// init counter for automatic page change
	auto_jump_counter = 0;
	/* state for warning picture */
	warning_state = 0;
	// set new active page
	active_page = page;
	// not element is touched
	touch_function = NULL;
	active_element = NULL;
	active_element_state = 0;

	// redraw screen contents: poll all components and lay them out on the screen
	p = get_page_descriptor (page);
	page_table = (_PAGE_DESCRIPTOR_t*) p;
	element_count = page_table->element_count;

	// skip page descriptor
	p += sizeof (_PAGE_DESCRIPTOR_t);

	// iterate all page elements
	for (i = 0; i < element_count; i++) {
		page_element = (_PAGE_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);

		switch (page_element->element_type) {
			case PAGE_ELEMENT_TYPE_PICTURE:
				draw_picture_element (p);
			break;
			case PAGE_ELEMENT_TYPE_JUMPER:
				draw_jumper_element (p);
			break;
			case PAGE_ELEMENT_TYPE_BUTTON:
				draw_button_element (p);
			break;
			case PAGE_ELEMENT_TYPE_BACKGROUND:
				fill_screen (p);
			break;
			case PAGE_ELEMENT_TYPE_LED:
				draw_led_element (p);
			break;
			case PAGE_ELEMENT_TYPE_VALUE:
				draw_value_element (p);
			break;
			case PAGE_ELEMENT_TYPE_SBUTTON:
				draw_sbutton_element (p, 0);
			break;
#ifdef LCD_DEBUG
			default: printf_P (PSTR("unknown page element %d\n"), page_element->element_type);
#endif
		}

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		p += page_element->element_size;
	}

}

// checks active page components on touch event
void page_touch_event (t_touch_event* evt) {
	
char* p;
_PAGE_DESCRIPTOR_t	*page_table;
_PAGE_ELEMENT_t		*page_element;
uint8_t	element_count;
int i;

	if (!flash_content_ok) {
		touch_function = NULL;
		active_element = NULL;
		active_element_state = 0;
		return;
	}
	// reset time for automatic page change
	auto_jump_counter = 0;
	// do we already have an actively touched element?
	if (active_element && touch_function) {

		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		if ((*touch_function)(active_element, evt, &active_element_state )) {
			// release focus
			touch_function = NULL;
			active_element = NULL;
			active_element_state = 0;
		}
		return;
	}

	// poll all components of active page and check, if they match the touched area
	p = get_page_descriptor (active_page);
	page_table = (_PAGE_DESCRIPTOR_t*) p;
	element_count = page_table->element_count;

	// skip page descriptor
	p += sizeof (_PAGE_DESCRIPTOR_t);

	// iterate all page elements
	for (i = 0; i < element_count; i++) {
		page_element = (_PAGE_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		
		switch (page_element->element_type) {
			case PAGE_ELEMENT_TYPE_PICTURE:
				// skip, pictures have no activity
			break;
			case PAGE_ELEMENT_TYPE_JUMPER:
				if (!touch_jumper_element (p, evt, &active_element_state)) {
					touch_function = touch_jumper_element;
					active_element = p;
					return;
				}
				// for safety
				active_element_state = 0;
			break;
			case PAGE_ELEMENT_TYPE_BUTTON:
				if (!touch_button_element(p, evt, &active_element_state)) {
					touch_function = touch_button_element;
					active_element = p;
					return;
				}
				// for safety
				active_element_state = 0;
			break;
			case PAGE_ELEMENT_TYPE_LED:
				if (!touch_led_element(p, evt, &active_element_state)) {
					touch_function = touch_led_element;
					active_element = p;
					return;
				}
				// for safety
				active_element_state = 0;
			break;
			case PAGE_ELEMENT_TYPE_SBUTTON:
				if (!touch_sbutton_element(p, evt, &active_element_state)) {
					touch_function = touch_sbutton_element;
					active_element = p;
					return;
				}
				// for safety
				active_element_state = 0;
			break;
		}

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		p += page_element->element_size;
	}

}


/**
* @brief processes a new group message received from the EIB
*
*/
void lcd_page_process_msg (uint16_t address) {

char* p;
_PAGE_DESCRIPTOR_t	*page_table;
_PAGE_ELEMENT_t		*page_element;
uint8_t	element_count;
int i;
int	eib_object;

	if (!flash_content_ok)
		return;

	if (system_page_active || is_screen_locked() )
		return;

	eib_object = get_group_adress_index (address);
	if (eib_object < 0)
		return;

	// poll all components of active page and check, if they match the eib address
	p = get_page_descriptor (active_page);
	page_table = (_PAGE_DESCRIPTOR_t*) p;
	element_count = page_table->element_count;

	// skip page descriptor
	p += sizeof (_PAGE_DESCRIPTOR_t);

	// iterate all page elements
	for (i = 0; i < element_count; i++) {
		page_element = (_PAGE_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		
		switch (page_element->element_type) {
			case PAGE_ELEMENT_TYPE_PICTURE:
			case PAGE_ELEMENT_TYPE_JUMPER:
			case PAGE_ELEMENT_TYPE_BACKGROUND:
				// skip, no activity
			break;
			case PAGE_ELEMENT_TYPE_BUTTON:
			break;
			case PAGE_ELEMENT_TYPE_LED:
				check_led_element (p, eib_object);
			break;
			case PAGE_ELEMENT_TYPE_VALUE:
				check_value_element (p, eib_object);
			break;
			case PAGE_ELEMENT_TYPE_SBUTTON:
				check_sbutton_element (p, eib_object, active_element==p, active_element_state);
			break;
#ifdef LCD_DEBUG
			default: printf_P (PSTR("unknown page element %d\n"), page_element->element_type);
#endif
		}

		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		p += page_element->element_size;
	}


}

void recover_active_page () {
	set_page (active_page);
}

uint8_t get_active_page () {
	return active_page;
}


void page_time_ticker (void) {

char* p;
_PAGE_DESCRIPTOR_t	*page_table;
_PAGE_ELEMENT_t		*page_element;
uint8_t	element_count;
uint8_t	keep_warning_sound;
int i;

	// check tick counter
	if (auto_jump_counter < 0xff)
		auto_jump_counter++;
	warning_state = (warning_state +1) & 0x01;
	keep_warning_sound = 0; 

	// poll all components of active page and check all jumpers and warning LED
	p = get_page_descriptor (active_page);
	page_table = (_PAGE_DESCRIPTOR_t*) p;
	element_count = page_table->element_count;

	// skip page descriptor
	p += sizeof (_PAGE_DESCRIPTOR_t);

	// iterate all page elements
	for (i = 0; i < element_count; i++) {
		page_element = (_PAGE_ELEMENT_t*) p;

		// set page descriptions bank for safety
		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		
		switch (page_element->element_type) {
			case PAGE_ELEMENT_TYPE_JUMPER:
				if (auto_check_jumper_element (p, auto_jump_counter)) {
					return;
				}
			break;
			case PAGE_ELEMENT_TYPE_PICTURE:
			case PAGE_ELEMENT_TYPE_BACKGROUND:
			case PAGE_ELEMENT_TYPE_BUTTON:
			break;
			case PAGE_ELEMENT_TYPE_LED:
				if (check_led_warning_state (p, warning_state)) {
					set_backlight_on ();
					auto_jump_counter = 0;
					keep_warning_sound = 1;
				}
			break;
			case PAGE_ELEMENT_TYPE_VALUE:
			case PAGE_ELEMENT_TYPE_SBUTTON:
			break;
#ifdef LCD_DEBUG
			default: printf_P (PSTR("unknown page element %d\n"), page_element->element_type);
#endif
		}

		XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
		p += page_element->element_size;
	}

	if (!keep_warning_sound)
		sound_terminate_repetitions ();

}


// called every 30ms from main
void process_cyclic_page_events (void) {

	if (t_divider++ < 33)
		return;

	t_divider = 0;

	if (!is_system_page_active ())
		page_time_ticker ();
	else 
		auto_jump_counter = 0;
}
