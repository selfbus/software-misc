/** \file ScreenCtrl.h
 *  \brief Constants and definitions for the system pages
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef SCREEN_CTRL_H_
#define SCREEN_CTRL_H_

#include "tft_io.h"
#include "TPUart.h"
#include "dht11.h"

// these are the implemented system pages
#define SYSTEM_PAGE_NONE				0	// no system page is active
#define SYSTEM_PAGE_MAIN				1	// main system page is active
#define SYSTEM_PAGE_DOWNLOAD_SELECTION	2	// download file selection list
#define SYSTEM_PAGE_DOWNLOAD_PROGRESS	3	// download progress page is open
#define SYSTEM_PAGE_BUSMON				4	// busmon page is open
#define SYSTEM_PAGE_BUSMON_PAUSED		5	// busmon page is paused
#define SYSTEM_PAGE_MONITOR_SELECTION	6	// page to select monitor function
#define SYSTEM_PAGE_HARDWARE_MONITOR	7	// hardware monitor page (IR, Buttons, ...)
#define SYSTEM_PAGE_FLASH_CONTROL		8	// flash erase page (erase external flash)
#define SYSTEM_PAGE_REBOOT_CONFIRM		9	// confirm system reboot

#define	BYTE2COLOR(red, green, blue) ( ((red) & 0xf8) << 8) | ( ((green) & 0xfc) << 3) | (((blue) & 0xf8) >> 3)

extern uint8_t system_page_active;

// process a touch event
void process_touch_event (t_touch_event*);

void show_erase_progress (uint8_t);
void show_download_progress (uint32_t);
void remove_download_progress(void);
void init_download_progress(uint32_t);
void check_screen_lock(void);
void init_screen_control (void);
void busmon_show (t_eib_frame *);
void hwmon_show_ir_event (void);
void hwmon_show_ds1820_event (double, int8_t, uint8_t, uint8_t, uint8_t);
void hwmon_show_dht_event (double, double, double, int8_t, int8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void hwmon_show_button_event (uint8_t, uint8_t);
uint8_t is_screen_locked (void);
uint8_t is_system_page_active (void);

void create_system_info_screen (void);
void create_screen_lock (void);


#endif //SCREEN_CTRL_H_

