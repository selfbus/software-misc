/**
 * \file System.h
 *
 * \brief System constants for the EIB-LCD Controller Firmware
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <cfg/os.h>
#include <compiler.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>

#include "task.h"
#include "hardware.h"
#include "TPUart.h"
#include "EIBLayers.h"
#include "EIBObjects.h"
#include "MemoryMap.h"
#include "NandFlash.h"
#include "ScreenCtrl.h"
#include "page.h"
#include "picture.h"
#include "addr_tab.h"
#include "listen.h"
#include "cyclic.h"

#include <dev/board.h>
//#include <dev/adc.h>
#include <dev/watchdog.h>
#include <dev/nplmmc.h>

#include <sys/version.h>
#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/heap.h>
#include <sys/event.h>

#include "tft_io.h"
#include "Sound.h"
#include "rc5_io.h"

#ifdef NUTDEBUG
#include <sys/osdebug.h>
#include <net/netdebug.h>
#endif

#define USE_WATCHDOG
#define WDT_INIT_TIME	1000	// max. timeout is 1.9s
/**< Max. allowed time interval for the WDT retrigger */
#define WDT_TIME		1000

#define MAIN_TIME_LOOP_SLEEP	30		// 30ms sleep time for main timer loop
#define SCREEN_LOCK_TIME	(33*10)		// 10sec/MAIN_TIME_LOOP_SLEEPms

/**
 * \brief Definitions for Bootloader
 */
typedef struct
{
	unsigned long dev_id;		/**< Device ID. In our case fixed to use the same firmware image independent of the display type */
	unsigned short app_version; /**< Application Version: Main.Sub */
	unsigned short crc;			/**< crc checksum to verify the consistency of the image */
} bootldrinfo_t;


#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define min(a,b)  ( (a)<(b) ? (a) : (b) )
#define max(a,b)  ( (a)>(b) ? (a) : (b) )
#define I2M(u) ((((u)>>8)&0xff) | (((u)&0xff)<<8))

#define CONFEIBLCD_EE_OFFSET	CONFNET_EE_OFFSET + sizeof(CONFNET)

#define CONFEIBLCD_MAGIC		(('E' << 8) | 'D')

/**
 * \brief Header struct of project file
 */
typedef struct __attribute__ ((packed)) {
char 		magic[6];
uint8_t		file_structure_version;
uint16_t	physical_address;
uint8_t		display_orientation;
uint8_t		display_dimming;
char 		source_file_name[128];
time_t		file_creation_date;
char		file_comment[40];
uint8_t		checksum;
} _LCD_FILE_HEADER_t;

//#define LCD_HAEADER_SIZE			182 // FIXME: better sizeof header?
#define LCD_HAEADER_SIZE			(sizeof (_LCD_FILE_HEADER_t))
#define LCD_HEADER_MAGIC_ADDR_0		0, 0x00
#define LCD_HEADER_MAGIC_ADDR_1		0, 0x01
#define LCD_HEADER_MAGIC_ADDR_2		0, 0x02
#define	LCD_HEADER_MAGIC_0			0x4945
#define	LCD_HEADER_MAGIC_1			0x4C42
#define	LCD_HEADER_MAGIC_2			0x4443
// V1.0
#define LCD_HEADER_VERSION_ADDR		0, 0x03
#define LCD_VERSION_EXPECTED		0x1D
// Physical Address
#define LCD_HEADER_PHYSICAL_ADDR_LB	0, 0x03
#define LCD_HEADER_PHYSICAL_ADDR_HB	0, 0x04
// Display orientation
#define LCD_HEADER_ORIENTATION		0, 0x04
// Display backlight dimming
#define LCD_HEADER_DIMMING			0, 0x05
// TOC
#define	LCD_TOC_ADDR				0,LCD_HAEADER_SIZE
#define TOC_HEADER_SIZE				2
#define TOC_ITEMS_SIZE				9

typedef struct __attribute__ ((packed)) {
uint8_t		type;
uint32_t	flash_position;
uint32_t	size;
} _LCD_FILE_TOC_ENTRY_t;

#define MAX_FNAME_LENGTH	32
#define MAX_NAME_LENGTH		13
#define MAX_FNAME_COUNT		12
typedef struct __attribute__ ((packed)) {
char		fname[MAX_FNAME_LENGTH];
char		fname83[MAX_NAME_LENGTH];
uint32_t	size;
} _LCD_FILE_NAMES_t;

#define	XRAM_SELECT_BLOCK(blk)		OUTB(CPLD_BASE_ADDR + RAM_BANK_ADDR, blk)
#define	XRAM_GET_SELECTED_BLOCK		INB(CPLD_BASE_ADDR + RAM_BANK_ADDR)
#define XRAM_SECTOR_SIZE			0x2000
#define XRAM_MAX_SECTOR				61

#define INB(reg)         (*((volatile uint8_t *) reg))
#define OUTB(reg, val)   (*((volatile uint8_t *) reg) = val)

//EIB_LCD.c
extern const bootldrinfo_t bootlodrinfo;
//EIB_LCD.c
extern uint8_t flash_content_ok;


/* DeviceCtrl.c */
extern unsigned char select_state;
void save_setup (void);
unsigned char calc_checksum (char *p, int	sz);
short store_to_NVMEM (char *data, short len, short NVOffset);
short read_from_NVMEM (char *data, short len, short NVOffset);

/* System.c */
//0=horizontal, 1=90�left, 2=90�right, 3=180�
#define DISPLAY_ORIENTATION_HOR		0
#define DISPLAY_ORIENTATION_90L		1
#define DISPLAY_ORIENTATION_90R		2
#define DISPLAY_ORIENTATION_UPSIDE	3

extern volatile uint8_t display_orientation;
// copies data from Flash to xram. Can NOT handle sector overflows!
void copy_Flash_to_XRAM (uint8_t, uint16_t, uint8_t, uint16_t, uint16_t);
// init hardware to default state
void init_hardware (void);
// init block device for SD card read
void init_sd_card (void);
// evaluate flash contents and init system
uint8_t	init_system_from_flash (void);
// tries to download config file from SD card
uint8_t	download_from_sd_card (void);
// mounts SD card.
// Returns 1 on success, 0 on fail
int	mount_SD_card (void);
// unmounts SD card
void unmont_SD_card (void);
// gets a list of directory entries
uint8_t	get_list_of_lcd_files (_LCD_FILE_NAMES_t**);
// download selected file from SD card
uint8_t download_file_from_sd_card (_LCD_FILE_NAMES_t*);
// set physical address of device
void init_physical_address_from_Flash (void);
// invalidates contents of the Flash
void set_flash_content_invalid(void);



#endif // _SYSTEM_H_
