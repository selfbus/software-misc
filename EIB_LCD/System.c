/**
 * \file System.c
 *
 * \brief System utility functions for the EIB-LCD Controller Firmware
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "System.h"
#include "FATSingleOpt/dos.h"
#include <avr/wdt.h>

volatile uint8_t display_orientation;
const uint8_t port_bit[7] = { 0x01, 0x02, 0x04, 0x10, 0x20, 0x40, 0x80 };
//"PE0 (1)", "PE1 (9)", "PE2 (LP)", "PF4 (4)", "PF5 (7)", "PF6 (5)", "PF7 (6)"
const char channel_names[7][4] = { "PE0", "PE1", "PE2", "PF4", "PF5", "PF6", "PF7" };


// copies data from Flash to xram. Can NOT handle sector overflows!
// stores data starting from 1st XRAM Bank address
void copy_Flash_to_XRAM (uint8_t flash_sector, uint16_t flash_offset, uint8_t xram_block, uint16_t xram_offset, uint16_t byte_count) {

uint16_t 	fdata;
volatile uint8_t*	p;


	if (!byte_count)
		return;

	p = (uint8_t*)(xram_offset+XRAM_BASE_ADDRESS);
	// set xram page
	XRAM_SELECT_BLOCK (xram_block);

	// check, if starting on odd byte address
	if (flash_offset & 0x01) {
		fdata = read_flash (flash_sector, (flash_offset >> 1) & 0x7fff);
		*p++ = (fdata >> 8) & 0xff;
		byte_count--;
	}
	flash_offset = ((flash_offset+1) >> 1) & 0x7fff;

	while (byte_count) {
		// get flash contents
		fdata = read_flash (flash_sector, flash_offset++);
		// write xram data
		*p++ = fdata & 0xff;
		if (--byte_count) {
			byte_count--;
			*p++ = (fdata >> 8) & 0xff;
		}
	}
}


// invalidates contents of the Flash
void set_flash_content_invalid() {
	flash_content_bad = 1;
}

// init hardware to clear settings from bootloader or hardware usage of last LCD project
void init_hardware (void) {

	DDRB = 0;
	PORTB = 0;
	DDRF = 0;
	PORTF = 0;
	DDRG = 0;
	PORTG = 0;
	SPCR = 0;
	SPSR = 0;

	backlight_dimming = 25;
//	backlight_active = 200;     // Reduce startup current of SMPS
backlight_active = 127;     // Reduce startup current of SMPS

}

void init_hardware_objects (void) {

	INIT_PORT_PE
	INIT_PORT_PF

}


// init block device for SD card read
void init_sd_card (void) {

	// init SD hardware
 	MMC_IO_Init();
}


// evaluate flash contents and init system
int8_t	init_system_from_flash (void) {

uint8_t	toc_items;
uint8_t tft_config;

	// check Flash contents
	flash_content_bad = 1;
	toc_items = 0;

	// check magic
	if ((read_flash (LCD_HEADER_MAGIC_ADDR_0) != LCD_HEADER_MAGIC_0) ||
		(read_flash (LCD_HEADER_MAGIC_ADDR_1) != LCD_HEADER_MAGIC_1) ||
		(read_flash (LCD_HEADER_MAGIC_ADDR_2) != LCD_HEADER_MAGIC_2) ) {
        flash_content_bad = 2;
		return -2;
    }
	// check version
	  else if (( read_flash (LCD_HEADER_VERSION_ADDR) & 0xff) != LCD_VERSION_EXPECTED) {
        flash_content_bad = 3;
		return -3;
    }
	// check TOC entries
	  else {
		// get TFT configuration from Flash: d7: invert x, d6: invert y, d5: rotation
        // d3-2: Display Type, d1-0: display orientation
		// d3-2:	00: 320x240
		//			01: 800x480
		//			10:
		tft_config = (read_flash (LCD_HEADER_ORIENTATION) >> 8) & 0xff;
		// get display orientation from Flash
		display_orientation = tft_config & 0x03;
		// invert X coordinate of touch position
		invert_touch_x = (tft_config & 0x80) > 0;
		// invert Y coordinate of touch position
		invert_touch_y = (tft_config & 0x40) > 0;
        drv_lcd_rotate( ((tft_config & 0x20) > 0));

		printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_BLACK, PSTR("Touch mirror (x/y): %d/%d"), invert_touch_x, invert_touch_y);

		// get backlight dimming from Flash (not used any more)
        //read_flash ( LCD_HEADER_DIMMING) & 0xff
		// copy TOC to xram
		toc_items = read_flash (LCD_TOC_ADDR >> 1) & 0xff;
		copy_Flash_to_XRAM (LCD_TOC_ADDR, XRAM_TOC_ADDR, TOC_HEADER_SIZE + toc_items*TOC_ITEMS_SIZE);
		// check TOC contents
int i;
_LCD_FILE_TOC_ENTRY_t	*toc;
		toc = (_LCD_FILE_TOC_ENTRY_t*) (TOC_HEADER_SIZE + XRAM_BASE_ADDRESS);
		for (i = 0; i < toc_items; i++) {
			XRAM_SELECT_BLOCK(XRAM_TOC_PAGE);
uint16_t foffset;
uint8_t fpage;
fpage = (toc->flash_position >> 16) & 0xffff;
foffset = (toc->flash_position >> 1) & 0x7fff;
			printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_BLACK, PSTR("type: %d at (%x) %x"), toc->type, fpage, foffset);

			switch (toc->type) {
				// address table
				case 1:
					// copy address table into RAM mirror
					printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_BLACK, PSTR("move address table returned: %d"), move_address_table (toc->flash_position, toc->size));
				break;
				// page descriptions
				case 3:
					// copy page descriptions into RAM mirror
					printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_BLACK, PSTR("move page description returned: %d"), move_page_descriptions (toc->flash_position, toc->size));
				break;
				// picture table
				case 4:
					set_picture_table_start_address (toc->flash_position);
				break;
				// sound table
				case 5:
					set_sound_table_start_address (toc->flash_position);
				break;
				case 6:
					// copy listen elements descriptions into RAM mirror
					printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_BLACK, PSTR("move listen element description returned: %d"), move_listen_descriptions (toc->flash_position, toc->size));
				break;
				case 7:
					// copy cyclic elements descriptions into RAM mirror
					printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_BLACK, PSTR("move cyclic element description returned: %d"), move_cyclic_descriptions (toc->flash_position, toc->size));
				break;
				default:
					printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_RED, PSTR("unknown type %d"), toc->type);
			}
			toc++;
		}

		// Flash content valid now
        flash_content_bad = 0;
		// set all EIB objects to 0
		eib_object_init ();
		// init hardware
        // TODO: Do we need this here? Isn't this done by element init?
		lcd_init_listen_objects ();
		lcd_init_cyclic_objects ();
	}

	return 0;
}

// set physical address of device FIXME: ugly due to non aligned access
void init_physical_address_from_Flash (void) {

uint16_t phys_addr;
	phys_addr = read_flash (LCD_HEADER_PHYSICAL_ADDR_LB);
	phys_addr = (phys_addr >> 8) & 0xff;
	phys_addr |= (read_flash (LCD_HEADER_PHYSICAL_ADDR_HB) << 8) & 0xff00;
	eib_set_device_address(EIB_DEVICE_CHANNEL, phys_addr);
}

/*
   	Determines LCD Module type from resistor coding.
	Tries to detect a resistor between data line Dn and Dn+1.
	No resistor -> Type 0
	Resistor between Dn and Dn+1 -> Type = n+1
	Only checks for the first resistor.

	CAUTION:
	Assumes the stack pointer and all used variables are in the internal SRAM!

*/
uint8_t check_lcd_type_code (void) {

	uint8_t	tft_type = 0;
	uint8_t n;
	uint8_t d;
	uint8_t _portg, _ddrg, _porta, _ddra;

	NutEnterCritical();

	_portg = PORTG;
	_ddrg = DDRG;
	_porta = PORTA;
	_ddra = DDRA;

	// disable external memory i/f
	PORTG |= 0x03; //disable RDn, WRn
	PORTG &= ~(1 << 3); // disable ALE
	DDRG |= 0x07; // set Rdn, WRn, ALE to output

	MCUCR &= ~(1 << SRE);
	XMCRB &= 0xff ^(1 << XMBK);	// disable buskeeper

	// search for resistor (~10k)
	for (n = 0; n < 7; n++) {

		DDRA = (1 << n);
		PORTA = 0xff ^ (1 << n);

		// give time to settle
		for (d = 0; d < 40; d++)
			asm volatile ("nop");

		if (!(PINA & (1<<(n+1)) )) {
			tft_type = n+1;
			break;
		}
	}

	// enable external memory i/f
	PORTG = _portg;
	DDRG = _ddrg;
	PORTA = _porta;
	DDRA = _ddra;

	MCUCR |= (1 << SRE);
	XMCRB |= (1 << XMBK);	// enable buskeeper

	NutExitCritical();
	return tft_type;

}


// return: 0=ok, 1=file error, 2=out of mem, 3 out of heap
uint8_t download_file_from_sd_card (_LCD_FILE_NAMES_t *fname) {

	init_hardware_objects ();

	init_download_progress(fname->size);

	uint8_t result = file_2_nand_flash ( fname->fname83, 0);
	remove_download_progress();
	return result;
}

// this function tries to mount the SD card file system
// return value:
// 0: function failed
// 1: SD card volume is mounted
int	mount_SD_card (void) {

#ifdef LCD_DEBUG
    printf_P(PSTR("Try to get drive info\n"));
#endif
   	printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("Mounting SD Card"));

 	if(GetDriveInformation()!=F_OK) { // get drive parameters
#ifdef LCD_DEBUG
		puts("failed\n");
#endif
		printf_tft_P( TFT_COLOR_RED, TFT_COLOR_WHITE, PSTR("failed"));
		return 0;
	}

#ifdef LCD_DEBUG
   	puts("OK\n");
#endif
	printf_tft_P( TFT_COLOR_GREEN, TFT_COLOR_WHITE, PSTR("ok"));
	return 1;
}

void unmont_SD_card (void) {
//	if (sd_hvol != -1)
//		_close(sd_hvol);
}

uint8_t	get_list_of_lcd_files (_LCD_FILE_NAMES_t **fnames) {

uint8_t entries =0;
_LCD_FILE_NAMES_t *current_fname;


	if(Findfirst()) {//find FIRST file in directory
		do {
			if(ffblk.ff_attr==ATTR_FILE) { //did we find a file ?
				// check, if file is ".lcdb" file
				if (strstr_P (ffblk.ff_longname, PSTR(".lcdb"))) {
					entries++;
				}
			}
		}while(Findnext());
	}


	if (!entries)
		return entries;
	// avoid overflow
	if (entries > MAX_FNAME_COUNT)
		entries = MAX_FNAME_COUNT;
	// allocate memory for file names
	*fnames = malloc(sizeof (_LCD_FILE_NAMES_t) * entries);
	current_fname = *fnames;
	if (!current_fname) {
		return 0;
	}
	// redo directory parsing
	entries = 0;

   	Findfirst(); //find FIRST file in directory
    do {
		if(ffblk.ff_attr==ATTR_FILE) {//did we find a file ?
			//do we have a long filename ?
			// check, if file is ".lcdb" file
			if (strstr_P (ffblk.ff_longname, PSTR(".lcdb"))) {
				entries++;
				strlcpy (current_fname->fname, ffblk.ff_longname, MAX_FNAME_LENGTH);
				strlcpy (current_fname->fname83, ffblk.ff_name, MAX_NAME_LENGTH);
				current_fname->size = ffblk.ff_fsize;
				current_fname++;
			}
		}
	} while ((entries < MAX_FNAME_COUNT) && Findnext());

	return entries;
}

/* reboot the system by WDT overflow */
void reboot () {

	// disable interrupts
	NutEnterCritical ();

	// start WDT
	wdt_enable (WDTO_30MS);

	// wait for reset
	while (1);

}
