/** \file ScreenCtrl.c
 *  \brief Functions controlling fixed system pages
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display and control system information page
 *	- display and control project download page
 *	- display and control EIB busmonitor page
 *	- display and control hardware monitor function
 *	- display and control screen lock function
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "System.h"
#include "ScreenCtrl.h"

// Global Variables
volatile uint32_t filesize;
// 0 = none
// 1 = system setup man page
// 2 = system download page
// 3 = system download progress page
// 4 = system setup page
uint8_t	system_page_active;
uint8_t sd_files;
_LCD_FILE_NAMES_t *sd_file_names;
// timer for screen locking
volatile uint16_t screen_lock;
uint16_t	monitor_y;
uint16_t	monitor_color;

// Flash Control Page
uint8_t G_support_qfi = 0;	// True if FLASH supports QFI
uint8_t G_pri_address;		// Address for PRI table within QFI info

#define	BUTTON_WIDTH	100
#define	BUTTON_WIDTH_SMALL	45
#define	BUTTON_HEIGHT	30

#define	SETUP_BUTTON_XPOS		004
#define	SETUP_BUTTON_YPOS		204
#define	MONITOR_BUTTON_XPOS		4
#define	MONITOR_BUTTON_YPOS		204
#define	BUSMON_BUTTON_XPOS		4
#define	BUSMON_BUTTON_YPOS		204
#define PAUSE_BUTTON_XPOS		40
#define PAUSE_BUTTON_YPOS		204
#define RESUME_BUTTON_XPOS		40
#define RESUME_BUTTON_YPOS		204
#define HARDWARE_MONITOR_BUTTON_XPOS	109
#define HARDWARE_MONITOR_BUTTON_YPOS	204
#define	DOWNLOAD_BUTTON_XPOS	109
#define	DOWNLOAD_BUTTON_YPOS	204
#define CLRSCN_BUTTON_XPOS		40
#define CLRSCN_BUTTON_YPOS		204
#define	EXIT_BUTTON_XPOS		214
#define	EXIT_BUTTON_YPOS		204
#define	DOWNLOAD_LIST_UP_XPOS	004
#define DOWNLOAD_LIST_UP_YPOS	204
#define	DOWNLOAD_LIST_DOWN_XPOS	054
#define DOWNLOAD_LIST_DOWN_YPOS	204

// Flash Erase
#define ERASE_BUTTON_XPOS		4
#define ERASE_BUTTON_YPOS		160
#define FLASH_INFO_BUTTON_XPOS	4
#define FLASH_INFO_BUTTON_YPOS	204
#define FLASH_ERASE_BUTTON_XPOS_LONELY	70
#define FLASH_ERASE_BUTTON_XPOS 109
#define FLASH_ERASE_BUTTON_YPOS 204

#define CHARACTER_WIDTH			8
#define BUTTON_TEXT_OFFSET		8

// draws a simple button
void draw_button (uint16_t x_pos, uint16_t y_pos, uint8_t width, char *face) {

	// draw inner area
	tft_fill_rect (BYTE2COLOR (128,128,128), x_pos+1,y_pos+1,x_pos+width-1,y_pos+BUTTON_HEIGHT-1);

	// draw outer lines
	tft_fill_rect (BYTE2COLOR (0,0,0), x_pos,				y_pos,	x_pos+width,	y_pos+1);
	tft_fill_rect (BYTE2COLOR (0,0,0), x_pos,				y_pos,	x_pos+1,			y_pos+BUTTON_HEIGHT);
	tft_fill_rect (BYTE2COLOR (0,0,0), x_pos+width-1,y_pos,	x_pos+width,	y_pos+BUTTON_HEIGHT);
	tft_fill_rect (BYTE2COLOR (0,0,0), x_pos,	y_pos+BUTTON_HEIGHT-1,	x_pos+width,	y_pos+BUTTON_HEIGHT);

	// get length of text
	uint16_t len = strlen (face) * CHARACTER_WIDTH;
	// put text
	showzifustr(x_pos+(width - len)/2, y_pos+BUTTON_TEXT_OFFSET, (unsigned char*) face, BYTE2COLOR (0,0,255),BYTE2COLOR (128,128,128));
}

// 1 = hit, 0=not hit
uint8_t check_button (uint16_t x_pos, uint16_t y_pos, uint8_t width, t_touch_event *evt) {

	return 	(x_pos < evt->lx) && (y_pos < evt->ly) && (x_pos + width > evt->lx) && (y_pos + BUTTON_HEIGHT > evt->ly);

}

char* (*list_get_item_string) (uint8_t);
uint8_t	list_length;
uint8_t list_selected_item;
uint16_t list_x_pos;
uint16_t list_y_pos;
uint16_t list_w;
uint16_t list_h;
uint8_t	 list_lines_in_box;
#define	LIST_BACKGROUND_COLOR	BYTE2COLOR (20,20,20)
#define	LIST_SELECT_BOX_COLOR	BYTE2COLOR (150,150,150)
#define	LIST_SELECTED_BOX_COLOR	BYTE2COLOR (255,0,0)
#define	LIST_TEXT_CHAR_COLOR	BYTE2COLOR (255,255,255)
#define	LIST_TEXT_BACK_COLOR	BYTE2COLOR (150,150,150)
#define	LIST_TEXT_XOFS			8
#define LIST_LINE_HEIGHT		13
#define	LIST_Y_LINE_MARGIN		1
#define	LIST_SELECTION_BOX_X	1
#define	LIST_SELECTION_BOX_Y	4
#define	LIST_SELECTION_BOX_W	5
#define	LIST_SELECTION_BOX_H	5

void init_selection_list (char* (*get_list_item)(uint8_t), uint8_t length, uint8_t selected, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

	// set variables
	list_get_item_string = get_list_item;
	list_length = length;
	list_selected_item = selected;
	list_x_pos = x;
	list_y_pos = y;
	list_w = w;
	list_h = h;

	// clear paint area
	tft_fill_rect (LIST_BACKGROUND_COLOR, list_x_pos, list_y_pos, list_x_pos + list_w -1, list_y_pos + list_h -1);
	// draw contents
	int i;
	uint16_t line_y;
	line_y = list_y_pos + LIST_Y_LINE_MARGIN;
	for (i = 0; (i < list_length) && ((list_y_pos + list_h) > (line_y + LIST_LINE_HEIGHT + LIST_Y_LINE_MARGIN)); i++) {
		// draw selection box
		tft_fill_rect (LIST_SELECT_BOX_COLOR, list_x_pos + LIST_SELECTION_BOX_X, 							line_y + LIST_SELECTION_BOX_Y,
											  list_x_pos + LIST_SELECTION_BOX_X + LIST_SELECTION_BOX_W-1, 	line_y + LIST_SELECTION_BOX_Y + LIST_SELECTION_BOX_H -1);
		// show selected
		if (i == list_selected_item)
			tft_fill_rect (LIST_SELECTED_BOX_COLOR,	list_x_pos + LIST_SELECTION_BOX_X+1,						line_y + LIST_SELECTION_BOX_Y+1,
											  		list_x_pos + LIST_SELECTION_BOX_X + LIST_SELECTION_BOX_W -2, 	line_y + LIST_SELECTION_BOX_Y + LIST_SELECTION_BOX_H -2);
		// put text string
		showzifustr(list_x_pos+LIST_TEXT_XOFS,line_y, (unsigned char*) list_get_item_string (i), LIST_TEXT_CHAR_COLOR, LIST_TEXT_BACK_COLOR);

		// goto next line
		line_y += LIST_LINE_HEIGHT;
	}
}

void selection_list_update_selection(int step) {

uint16_t line_y;
int	new_list_selected_item;

	// calculate current line
	line_y = list_y_pos + LIST_Y_LINE_MARGIN + list_selected_item * LIST_LINE_HEIGHT;
	// clear selection
	tft_fill_rect (LIST_SELECT_BOX_COLOR, list_x_pos + LIST_SELECTION_BOX_X, 							line_y + LIST_SELECTION_BOX_Y,
										  list_x_pos + LIST_SELECTION_BOX_X + LIST_SELECTION_BOX_W-1, 	line_y + LIST_SELECTION_BOX_Y + LIST_SELECTION_BOX_H -1);
	// update selection
	new_list_selected_item = list_selected_item + step;
	if (new_list_selected_item >= list_length)
		new_list_selected_item = 0;
	if (new_list_selected_item < 0)
		new_list_selected_item = list_length -1;
	list_selected_item = new_list_selected_item;

	// calculate current line
	line_y = list_y_pos + LIST_Y_LINE_MARGIN + list_selected_item * LIST_LINE_HEIGHT;
	// mark selection
	tft_fill_rect (LIST_SELECTED_BOX_COLOR,	list_x_pos + LIST_SELECTION_BOX_X+1,						line_y + LIST_SELECTION_BOX_Y+1,
									  		list_x_pos + LIST_SELECTION_BOX_X + LIST_SELECTION_BOX_W-2, 	line_y + LIST_SELECTION_BOX_Y + LIST_SELECTION_BOX_H-2);
}

uint8_t selection_list_get_selected_item (void) {
	return list_selected_item;
}


#define	DOWNLOAD_PROGRESS_XPOS	10
#define	DOWNLOAD_PROGRESS_YPOS	120
#define	DOWNLOAD_PROGRESS_WIDTH	 300
#define	DOWNLOAD_PROGRESS_HEIGHT 20
#define	DOWNLOAD_BAR_XPOS	15
#define	DOWNLOAD_BAR_YPOS	129
#define	DOWNLOAD_BAR_WIDTH	290
#define	DOWNLOAD_BAR_HEIGHT 2

void show_erase_progress (uint8_t current) {

uint16_t progress;

	progress = (current * (uint16_t)DOWNLOAD_BAR_WIDTH) / filesize;
	tft_fill_rect (BYTE2COLOR (255,0,0), DOWNLOAD_BAR_XPOS, DOWNLOAD_BAR_YPOS,
						DOWNLOAD_BAR_XPOS+progress,
						DOWNLOAD_BAR_YPOS+DOWNLOAD_BAR_HEIGHT);
	printf_tft_absolute_P(DOWNLOAD_BAR_XPOS, DOWNLOAD_BAR_YPOS-30, TFT_COLOR_RED, TFT_COLOR_BLUE, PSTR("Sector %u erased"), current);
}

void show_download_progress (uint32_t downloadsize) {

uint32_t	progress;

	progress = (downloadsize * DOWNLOAD_BAR_WIDTH) / filesize;
	tft_fill_rect (BYTE2COLOR (255,255,0), DOWNLOAD_BAR_XPOS, DOWNLOAD_BAR_YPOS,
				 		DOWNLOAD_BAR_XPOS+progress,
						DOWNLOAD_BAR_YPOS+DOWNLOAD_BAR_HEIGHT);
	if( !(downloadsize%10240) )	// Print just every 10kbyte thus download speed is not reduced
		printf_tft_absolute_P(DOWNLOAD_BAR_XPOS, DOWNLOAD_BAR_YPOS-30, TFT_COLOR_RED, TFT_COLOR_WHITE, PSTR("Loading... %u kbyte"), downloadsize/1024);
}

void remove_download_progress() {

	tft_fill_rect (BYTE2COLOR (255,255,255), DOWNLOAD_PROGRESS_XPOS, DOWNLOAD_PROGRESS_YPOS,
									DOWNLOAD_PROGRESS_XPOS+DOWNLOAD_PROGRESS_WIDTH,
									DOWNLOAD_PROGRESS_YPOS+DOWNLOAD_PROGRESS_HEIGHT);
}

void init_download_progress(uint32_t size) {

	filesize = size;

	tft_fill_rect (BYTE2COLOR (100,100,100), DOWNLOAD_PROGRESS_XPOS, DOWNLOAD_PROGRESS_YPOS,
									DOWNLOAD_PROGRESS_XPOS+DOWNLOAD_PROGRESS_WIDTH,
									DOWNLOAD_PROGRESS_YPOS+DOWNLOAD_PROGRESS_HEIGHT);
}


void create_system_info_screen (void) {

uint16_t addr;

	tft_clrscr(TFT_COLOR_LIGHTGRAY);	// White before

    printf_tft_P( TFT_COLOR_BLUE, TFT_COLOR_WHITE, PSTR("Nut/OS %s "), NutVersionString());
    printf_tft_P( TFT_COLOR_BLUE, TFT_COLOR_WHITE, PSTR("Firmware V%d.%d"), pgm_read_byte_far((char*)&bootlodrinfo.app_version +1), pgm_read_byte_far((char*)&bootlodrinfo.app_version));
	printf_tft_P( TFT_COLOR_ORANGE, TFT_COLOR_WHITE, PSTR("Build %s"), __TIMESTAMP__);

	addr = eib_get_device_address(EIB_DEVICE_CHANNEL);
    printf_tft_P( TFT_COLOR_RED, TFT_COLOR_WHITE, PSTR("Phys. Addr %d.%d.%d"), (addr >> 4) & 0x0f, addr & 0x0f, (addr >> 8) & 0xff );
	if (display_orientation == DISPLAY_ORIENTATION_HOR)
    	printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("Horizontal"));
	else if (display_orientation == DISPLAY_ORIENTATION_90L)
    	printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("90 left"));
	else if (display_orientation == DISPLAY_ORIENTATION_90R)
    	printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("90 right"));
	else if (display_orientation == DISPLAY_ORIENTATION_UPSIDE)
    	printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("180"));

	if (eib_get_status() == EIB_NORMAL)
    	printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("EIB online"));
	else
    	printf_tft_P( TFT_COLOR_RED, TFT_COLOR_WHITE, PSTR("EIB offline"));

	printf_tft_P (TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("TFT Controller= %d, R00=%4.4x"), controller_type, controller_id, lcd_type);
	printf_tft_P (TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("R-Code %u,   Resolution %u x %u"), lcd_type, get_max_x()+1, get_max_y()+1);
	draw_button (ERASE_BUTTON_XPOS, ERASE_BUTTON_YPOS, BUTTON_WIDTH, "Erase Flash");		// added by sh
	draw_button (MONITOR_BUTTON_XPOS, MONITOR_BUTTON_YPOS, BUTTON_WIDTH, "Monitor");
	draw_button (DOWNLOAD_BUTTON_XPOS, DOWNLOAD_BUTTON_YPOS, BUTTON_WIDTH, "Download");
	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
	system_page_active = SYSTEM_PAGE_MAIN;
}


char* get_download_files (uint8_t i) {

_LCD_FILE_NAMES_t *fname;

	if (i >= sd_files)
		return "error";

	fname = sd_file_names;
	fname += i;
	return fname->fname;
}

// Create Flash Erase control page (SH)
void create_flash_control_page (void) {
	uint8_t temp;

	// clear page contents
	tft_clrscr(TFT_COLOR_BLUE);
	// write header
	showzifustr(75,1, (unsigned char*)"Project FLASH Erase", TFT_COLOR_YELLOW, TFT_COLOR_BLUE);

	tft_set_cursor(START_CHAR_X_POS, 30);
	printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_RED, PSTR("      ########  W A R N I N G  ########      "));
	printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_RED, PSTR("This will wipe your project FLASH memory!    "));
	printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_RED, PSTR("You need to reload a project from the SD-card"));
	printf_tft_P( TFT_COLOR_WHITE, TFT_COLOR_RED, PSTR("afterwards to restore a project.             "));

	// Check if device supports QFI
	if (read_flash_qfi_info(FLASH_QFI_QRY+0) == 'Q')
		if (read_flash_qfi_info(FLASH_QFI_QRY+1) == 'R')
			if (read_flash_qfi_info(FLASH_QFI_QRY+2) == 'Y')
				G_support_qfi = 1;	// Yes, it does

	tft_set_cursor(START_CHAR_X_POS, 90);
	if (G_support_qfi) {
		printf_tft_P( TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("Found QFI Info, reading..."));

		// Read some interesting info and plot
		// Device Size
		printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("Size %u kbyte"), 1<<(read_flash_qfi_info(FLASH_QFI_DEVICE_SIZE)-10));

		temp = read_flash_qfi_info(FLASH_QFI_VCC_MIN);
		printf_tft_P( TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("VCC Min %x.%uV"), temp>>4, temp &0x0F);

		temp = read_flash_qfi_info(FLASH_QFI_VCC_MAX);
		printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("VCC Max %x.%uV"), temp>>4, temp &0x0F);

		// Min buffer write timeout
		printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("Buffer Write Timeout %u us"), 1<<(read_flash_qfi_info(FLASH_QFI_T_MIN_BUF_W)));

		// Block erase timeout
		printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("Block Erase Timeout %u ms"), 1<<(read_flash_qfi_info(FLASH_QFI_T_BL_ERASE)));

		// Search Primary Extended Table
		G_pri_address = read_flash_qfi_info(FLASH_QFI_PRI_ADDRESS);
		if (G_pri_address)
			printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("Found PRI table Version %c.%c at 0x%x"),
				read_flash_qfi_info(G_pri_address+FLASH_PRI_VER_MAJOR), read_flash_qfi_info(G_pri_address+FLASH_PRI_VER_MINOR),G_pri_address);
	}
	else
		printf_tft_P(TFT_COLOR_RED, TFT_COLOR_BLUE, PSTR("Device doesn´t support QFI!"));

#ifdef LCD_DEBUG
	// Dump the QFI Area
	if (G_support_qfi) {
		uint8_t i;
		printf_P(PSTR("\nFLASH supports QFI:"));
		for (i=0x10;i<=0x50;i++) {
			printf_P(PSTR("\nQFI Address %x =\t %x") ,i, read_flash_qfi_info(i));
		}
	}
	else
		printf_P(PSTR("\nFlash doesn't support QFI!"));
#endif
	// Issue device reset command to leave QFI mode
	reset_flash_chip();

	// Place QFI DUMP Button if supported
	if(G_support_qfi) {
		draw_button (FLASH_ERASE_BUTTON_XPOS, FLASH_ERASE_BUTTON_YPOS, BUTTON_WIDTH, "Erase FLASH");
		draw_button (FLASH_INFO_BUTTON_XPOS, FLASH_INFO_BUTTON_YPOS, BUTTON_WIDTH, "QFI Info");
	}
	else
		draw_button (FLASH_ERASE_BUTTON_XPOS_LONELY, FLASH_ERASE_BUTTON_YPOS, BUTTON_WIDTH, "Erase FLASH");
	// EXIT Button
	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
	// set active system page
	system_page_active = SYSTEM_PAGE_FLASH_CONTROL;
}

void resume_busmon_page (void) {

	// write header
	showzifustr(75,1, (unsigned char*)"Busmon       ", TFT_COLOR_BLACK, TFT_COLOR_WHITE);

	draw_button (PAUSE_BUTTON_XPOS, PAUSE_BUTTON_YPOS, BUTTON_WIDTH, "Pause");
	system_page_active = SYSTEM_PAGE_BUSMON;
}

void create_monitor_selection_page (void) {

	// clear page contents
	tft_clrscr(TFT_COLOR_WHITE);

	// write header
	showzifustr(75,100, (unsigned char*)"Monitor selection", TFT_COLOR_BLACK, TFT_COLOR_WHITE);

	draw_button (BUSMON_BUTTON_XPOS, BUSMON_BUTTON_YPOS, BUTTON_WIDTH, "Busmon");
	draw_button (HARDWARE_MONITOR_BUTTON_XPOS, HARDWARE_MONITOR_BUTTON_YPOS, BUTTON_WIDTH, "Hardware");
	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");

	// set active system page
	system_page_active = SYSTEM_PAGE_MONITOR_SELECTION;
}

void create_hardware_monitor_page (void) {

	// clear page contents
	tft_clrscr(TFT_COLOR_WHITE);
	// write header
	showzifustr(75,1, (unsigned char*)"Hardware Monitor", TFT_COLOR_BLACK, TFT_COLOR_WHITE);

	draw_button (CLRSCN_BUTTON_XPOS, CLRSCN_BUTTON_YPOS, BUTTON_WIDTH, "Clear Screen");
	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
	// set active system page
	monitor_y = 15;
	monitor_color = TFT_COLOR_WHITE;
	system_page_active = SYSTEM_PAGE_HARDWARE_MONITOR;
}

void create_busmon_page (void) {

	// clear page contents
	tft_clrscr(TFT_COLOR_WHITE);

	// write header

	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
	// set active system page
	monitor_y = 15;
	monitor_color = TFT_COLOR_WHITE;
	resume_busmon_page ();
}

void create_busmon_paused_page (void) {
	// write header
	showzifustr(75,1, (unsigned char*)"Busmon paused", TFT_COLOR_WHITE, TFT_COLOR_RED);

	draw_button (RESUME_BUTTON_XPOS, RESUME_BUTTON_YPOS, BUTTON_WIDTH, "Resume");
	// set active system page
	system_page_active = SYSTEM_PAGE_BUSMON_PAUSED;
}



void create_download_selection_page (void) {

	// set active system page
	// needs to be done before! SD card check to avoid start of project on EXIT in case of an error
	system_page_active = SYSTEM_PAGE_DOWNLOAD_SELECTION;

	// clear page contents
	tft_clrscr(TFT_COLOR_WHITE);

	// write header
	showzifustr(75,1, (unsigned char*)"Select file for download", TFT_COLOR_BLACK, TFT_COLOR_WHITE);

	sd_files = 0;
	// mount SD card
	showzifustr(30,80, (unsigned char*)"Please wait", TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	showzifustr(30,94, (unsigned char*)"Mounting SD card...", TFT_COLOR_BLACK, TFT_COLOR_WHITE);
    if (!mount_SD_card ()) {
#ifdef LCD_DEBUG
        puts("failed\n");
#endif
		showzifustr(30,108, (unsigned char*)"Error, can't read from SD card!", TFT_COLOR_RED, TFT_COLOR_WHITE);
		draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
        return;
	}
	// get list of lcd files
	sd_files = get_list_of_lcd_files (&sd_file_names);
	if (!sd_files) {
		showzifustr(30,108, (unsigned char*)"No .lcdb file found!", TFT_COLOR_RED, TFT_COLOR_WHITE);
		draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
		return;
	}

	// setup selection list
	init_selection_list (&get_download_files, sd_files, 0, 10, 20, 300, 160);

	// create buttons
	draw_button (DOWNLOAD_LIST_UP_XPOS, DOWNLOAD_LIST_UP_YPOS, BUTTON_WIDTH_SMALL, "Up");
	draw_button (DOWNLOAD_LIST_DOWN_XPOS, DOWNLOAD_LIST_DOWN_YPOS, BUTTON_WIDTH_SMALL, "Down");
	draw_button (DOWNLOAD_BUTTON_XPOS, DOWNLOAD_BUTTON_YPOS, BUTTON_WIDTH, "Download");
	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
}

void create_download_progress_page (void) {

_LCD_FILE_NAMES_t *fname;
uint8_t s;

	// get selected file name
	s = selection_list_get_selected_item ();
	fname = sd_file_names+s;

	// clear page contents
	tft_clrscr(TFT_COLOR_WHITE);

	// write header
	showzifustr(80,1, (unsigned char*)"Download", TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	showzifustr(10,30, (unsigned char*)"File name:", TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	showzifustr(10,45, (unsigned char*) fname->fname, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	printf_tft_absolute_P(10,60, TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("Size: %u kbyte"), (fname->size)/1024);

	// get selected file name
	if (!download_file_from_sd_card (fname)) {
		showzifustr(130,60, (unsigned char*)"success!", TFT_COLOR_BLACK, TFT_COLOR_GREEN);
		tft_set_cursor (START_CHAR_X_POS, 80);
		init_system_from_flash ();
		init_physical_address_from_Flash ();
	}
	else {
		showzifustr(130,60, (unsigned char*)"fail!", TFT_COLOR_BLACK, TFT_COLOR_RED);
	}

	// release all allocated resources
	free (sd_file_names);
	unmont_SD_card();

	// show exit button
	draw_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, "Exit");
	// set active system page
	system_page_active = SYSTEM_PAGE_DOWNLOAD_PROGRESS;
}

void process_system_page_event (t_touch_event *evt) {

	if (evt->state == TOUCHED) {

		if (system_page_active == SYSTEM_PAGE_MAIN) {
			// System info main page
			// check, if flash erase button is hit
			if (check_button (ERASE_BUTTON_XPOS, ERASE_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_flash_control_page ();	// added by sh
			}
			// check, if busmon button is hit
			if (check_button (MONITOR_BUTTON_XPOS, MONITOR_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_monitor_selection_page ();
			}
			// check, if download button is hit
			if (check_button (DOWNLOAD_BUTTON_XPOS, DOWNLOAD_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_download_selection_page ();
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				system_page_active = SYSTEM_PAGE_NONE;
				set_page (0);
			}
		}
		else if (system_page_active == SYSTEM_PAGE_DOWNLOAD_SELECTION) {
			// System download page

			if (sd_files > 0) {

				// check, if UP button is hit
				if (check_button (DOWNLOAD_LIST_UP_XPOS, DOWNLOAD_LIST_UP_YPOS, BUTTON_WIDTH_SMALL, evt)) {
					sound_beep_on (0);
					selection_list_update_selection	(-1);
				}
				// check, if UP button is hit
				if (check_button (DOWNLOAD_LIST_DOWN_XPOS, DOWNLOAD_LIST_DOWN_YPOS, BUTTON_WIDTH_SMALL, evt)) {
					sound_beep_on (0);
					selection_list_update_selection	(+1);
				}

				// check, if download button is hit
				if (check_button (DOWNLOAD_BUTTON_XPOS, DOWNLOAD_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
					sound_beep_on (0);
					create_download_progress_page ();
				}
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				free (sd_file_names);
				unmont_SD_card();
				create_system_info_screen ();
			}
		}
		else if (system_page_active == SYSTEM_PAGE_DOWNLOAD_PROGRESS) {

			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_system_info_screen ();
			}
		}
		else if (system_page_active == SYSTEM_PAGE_BUSMON) {

			// check, if Pause button is hit
			if (check_button (PAUSE_BUTTON_XPOS, PAUSE_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_busmon_paused_page ();
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_system_info_screen ();
			}
		}
		else if (system_page_active == SYSTEM_PAGE_BUSMON_PAUSED) {

			// check, if Resume button is hit
			if (check_button (RESUME_BUTTON_XPOS, RESUME_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				resume_busmon_page ();
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_system_info_screen ();
			}
		}
		else if (system_page_active == SYSTEM_PAGE_MONITOR_SELECTION) {

			if (check_button (BUSMON_BUTTON_XPOS, BUSMON_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_busmon_page ();
			}
			if (check_button (HARDWARE_MONITOR_BUTTON_XPOS, HARDWARE_MONITOR_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_hardware_monitor_page ();
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_system_info_screen ();
			}
		}
		else if (system_page_active == SYSTEM_PAGE_HARDWARE_MONITOR) {

			// Clear HW Monitor screen by rewriting page
			if (check_button (CLRSCN_BUTTON_XPOS, CLRSCN_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_hardware_monitor_page();
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_system_info_screen ();
			}
		}
		else if (system_page_active == SYSTEM_PAGE_FLASH_CONTROL) {

			// check, if Flash Erase button is hit
			if (check_button (FLASH_ERASE_BUTTON_XPOS, FLASH_ERASE_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
			#ifdef HW_DEBUG
				printf_P(PSTR("\nFLASH Erase started..."));
			#endif
				// Clear Screen
				tft_fill_rect(TFT_COLOR_BLUE, START_CHAR_X_POS, 20, END_CHAR_X_POS, END_CHAR_Y_POS);
				tft_set_cursor(15, 80);
				printf_tft_P(TFT_COLOR_RED, TFT_COLOR_BLUE, PSTR("Erasing FLASH..."));
				// Erase all blocks (128)
				uint8_t success = erase_complete_flash(FLASH_MAX_SECTOR, 0);

				tft_set_cursor(70, 145);
				if (!success) {
					printf_tft_P( TFT_COLOR_RED, TFT_COLOR_GREEN, PSTR("Successfully erased Flash!"));
				}
				else {
					printf_tft_P( TFT_COLOR_RED, TFT_COLOR_BLACK, PSTR("Something went wrong!"));
				}
			}
			// check, if QFI dump button is visible and hit
			if (G_support_qfi && check_button (FLASH_INFO_BUTTON_XPOS, FLASH_INFO_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);

				tft_fill_rect(TFT_COLOR_BLUE, START_CHAR_X_POS, 20, END_CHAR_X_POS, END_CHAR_Y_POS);
				tft_set_cursor(START_CHAR_X_POS, 20);
				// If Primary Extended Table exists plot some infos
				if (G_pri_address) {
					// PRI is around 0x10 long, depends on device
					printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("Found PRI table Version %c.%c at 0x%x"),
					read_flash_qfi_info(G_pri_address+FLASH_PRI_VER_MAJOR), read_flash_qfi_info(G_pri_address+FLASH_PRI_VER_MINOR),G_pri_address);
				}

				// Print the HEX dump
				printf_tft_P(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, PSTR("QFI HEX Dump:"));
				uint8_t i;	// address
				uint8_t j;	// line count
				for (i=0x10, j=0; i<0x60; i++) {
					if (i%8 == 0) {
						tft_set_cursor(START_CHAR_X_POS, 50+j*15);
						printf_tft_append(TFT_COLOR_ORANGE, TFT_COLOR_BLUE, "%#2.2X|", i);
						tft_set_cursor(35, 50+j*15);
						j++;	// Next Line
					}
					printf_tft_append(TFT_COLOR_YELLOW, TFT_COLOR_BLUE, " %#2.2x", read_flash_qfi_info(i));
				}
				// Issue device reset command to leave QFI mode
				reset_flash_chip();
			}
			// check, if Exit button is hit
			if (check_button (EXIT_BUTTON_XPOS, EXIT_BUTTON_YPOS, BUTTON_WIDTH, evt)) {
				sound_beep_on (0);
				create_system_info_screen ();
			}
		}
	}
}

void check_screen_lock() {

	if (screen_lock) {
		screen_lock--;
		if (screen_lock) {
			//update progress bar
			show_download_progress (SCREEN_LOCK_TIME - screen_lock);
		}
		else {
			//jump to previous page
			sound_beep_on (0);
			system_page_active = SYSTEM_PAGE_NONE;
			recover_active_page ();
		}
	}
}

void create_screen_lock () {

	screen_lock = SCREEN_LOCK_TIME;
	tft_clrscr (TFT_COLOR_DS_BLUE);
	showzifustr(80,1, (unsigned char*)"Screen is locked", TFT_COLOR_RED, TFT_COLOR_DS_BLUE);
	init_download_progress (SCREEN_LOCK_TIME);
}

void process_touch_event (t_touch_event* evt) {

	//ignore touch events while screen is locked
	if (screen_lock)
		return;

	// Did the user touch the system control area of the display
	if (evt->lx < 0) {
		if (evt->state == TOUCHED) {
			if ((evt->ly < 80) && (!system_page_active)) {
				sound_beep_on (0);
				set_page (0);
			}
			else if ((evt->ly < 160) && (!system_page_active)){
				sound_beep_on (0);
				create_system_info_screen ();
			}
			else if ((evt->ly > 160) && (!system_page_active)) {
				//touch lock for wiping
				sound_beep_on (0);
				create_screen_lock ();
			}
		}
	}
	else {

		if (system_page_active)
			process_system_page_event (evt);
		else
			page_touch_event (evt);
	}
}

void init_screen_control () {

	screen_lock = 0;
	system_page_active = SYSTEM_PAGE_NONE;
}

void hwmon_goto_next_line (void) {

	monitor_y += 10;
	if (monitor_y > 190) {
		monitor_y = 15;
		if (monitor_color == TFT_COLOR_WHITE)
		monitor_color = TFT_COLOR_LIGHTGRAY;
		else monitor_color = TFT_COLOR_WHITE;
	}
}


void busmon_show (t_eib_frame* msg) {

int i;
unsigned int xp = 1;
#define BUFF_SIZE	5
char buffer[BUFF_SIZE];
int d;

	if (system_page_active != SYSTEM_PAGE_BUSMON)
		return;

	for (i=0; i < msg->len; i++) {
		d = msg->frame[i] & 0xff;
		vsprintf_P (buffer, PSTR("%.2X "), (int*)&d);
		xp = showzifustr(xp,monitor_y,(unsigned char*) buffer,TFT_COLOR_BLACK,monitor_color);
		if ((i == 1) || (i == 3))
			xp -= 4;
	}
	// clear until e/o line
	tft_fill_rect (monitor_color, xp,monitor_y,get_max_x(),monitor_y+10);

	hwmon_goto_next_line();
}


void hwmon_show_ir_event () {

	if (system_page_active != SYSTEM_PAGE_HARDWARE_MONITOR)
		return;

	// clear until e/o line
	tft_fill_rect (monitor_color, 0,monitor_y,get_max_x(),monitor_y+10);
//FIXME: find better way to clear this line
	tft_set_cursor (1, monitor_y);
	printf_tft_P (TFT_COLOR_BLACK,monitor_color,PSTR("RC5: A=%2.2d C=%2.2d           "), rc5_a, rc5_c);
	hwmon_goto_next_line ();
}


void hwmon_show_ds1820_event (double t, int8_t t_off, uint8_t c, uint8_t crc, uint8_t ok) {

char s[] = "+";
	if (system_page_active != SYSTEM_PAGE_HARDWARE_MONITOR)
		return;

	// clear until e/o line
	tft_fill_rect (monitor_color, 0,monitor_y,get_max_x(),monitor_y+10);
//FIXME: find better way to clear this line
	tft_set_cursor (1, monitor_y);

	if (!ok) {
		printf_tft_P (TFT_COLOR_BLACK,TFT_COLOR_ORANGE,PSTR("DS1820 (%s): no slave   "), channel_names[c] );
	}
	else if (!crc) {
		//printf_tft_P (TFT_COLOR_BLACK,monitor_color,PSTR("DS1820 (%s): %6.2f (%+4.1f) "), channel_names[c], t, ((double)t_off)/10);
		if (t_off < 0)
			s[0] = '-';
		printf_tft_P (TFT_COLOR_BLACK,monitor_color,PSTR("DS1820 (%s): %6.2f (%s%1u.%1u) "), channel_names[c], t, s, abs(t_off/10), abs(t_off%10) );
	}
	else {
		printf_tft_P (TFT_COLOR_BLACK,TFT_COLOR_RED,PSTR("DS1820 (%s): bad CRC    "), channel_names[c] );
	}

	hwmon_goto_next_line ();
}


// Shows DHT11 events
// Temperature, Humidity, Channel, CRC, Not_OK
// TO DO: Find a better way to always output the sign for t_off and h_off. %+.1f results to +- for negative numbers --> is this a BUG in WinAVR??
void hwmon_show_dht_event (double t, double h, double d, int8_t t_off, int8_t h_off, uint8_t c, uint8_t crc, uint8_t not_ok, uint8_t type) {

char st[] = "+";
char sh[] = "+";
	if (system_page_active != SYSTEM_PAGE_HARDWARE_MONITOR)
	return;

	// clear until e/o line
	tft_fill_rect (monitor_color, 0,monitor_y,get_max_x(),monitor_y+10);
//FIXME: find better way to clear this line
	tft_set_cursor (1, monitor_y);

	if (not_ok==255) {
		printf_tft_P (TFT_COLOR_BLACK,TFT_COLOR_BLUE,PSTR("DHT?? (%s): Unsupported Sensor selected!  "), channel_names[c]);
	}
	else if (not_ok) {
		printf_tft_P (TFT_COLOR_BLACK,TFT_COLOR_ORANGE,PSTR("DHT%s (%s): no slave - error %d  "), dht_names[type], channel_names[c], not_ok);
	}
	else if (!crc) {
		if (t_off < 0)
			st[0] = '-';
		if (h_off < 0)
			sh[0] = '-';
		//printf_tft_P (TFT_COLOR_BLACK,monitor_color,PSTR("DHT%s (%s): T%5.1f(%+4.1f) H%5.1f(%+4.1f) D%5.1f"), dht_names[type], channel_names[c], t, ((double)t_off)/10, h, ((double)h_off)/10, d);
		printf_tft_P (TFT_COLOR_BLACK,monitor_color,PSTR("DHT%s (%s): T%5.1f(%s%1u.%1u) H%5.1f(%s%1u.%1u) D%5.1f"), dht_names[type], channel_names[c], t, st, abs(t_off/10), abs(t_off%10), h, sh, abs(h_off/10), abs(h_off%10), d);
	}
	else {
		printf_tft_P (TFT_COLOR_BLACK,TFT_COLOR_RED,PSTR("DHT%s (%s): bad CRC %x  "), dht_names[type], channel_names[c], crc );
	}

	hwmon_goto_next_line ();
}


void hwmon_show_button_event (uint8_t btn, uint8_t state) {

	if (system_page_active != SYSTEM_PAGE_HARDWARE_MONITOR)
		return;

	// clear until e/o line
	tft_fill_rect (monitor_color, 0,monitor_y,get_max_x(),monitor_y+10);
//FIXME: find better way to clear this line
	tft_set_cursor (1, monitor_y);
	printf_tft_P (TFT_COLOR_BLACK,monitor_color,PSTR("Btn: %d -> %d           "), btn, state);

	hwmon_goto_next_line ();
}

uint8_t is_system_page_active () {
	return (system_page_active != SYSTEM_PAGE_NONE) || is_screen_locked ();
}

uint8_t is_screen_locked () {
	return (screen_lock > 0);
}
