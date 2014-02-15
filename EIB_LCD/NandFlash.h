/** \file NandFlash.h
 *  \brief Constants and macros for the Nand Flash memory control
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _NAND_FLASH_H_
#define _NAND_FLASH_H_

#include "System.h"

// define IO pins for Flash control
#define	FLASH_RESET_DDR		DDRD
#define FLASH_RESET_PORT	PORTD
#define FLASH_RESET_BIT		7
#define	FLASH_BUSY_DDR		DDRD
#define FLASH_BUSY_PORT		PIND
#define FLASH_BUSY_PORT_PU	PORTD
#define	FLASH_BUSY_BIT		6

#define	INIT_FLASH_RESET			DDRD |= (1 << FLASH_RESET_BIT);
#define	INIT_FLASH_BUSY				DDRD &= 0xff ^ (1 << FLASH_BUSY_BIT); FLASH_BUSY_PORT_PU = 1 << FLASH_BUSY_BIT;
#define	SET_FLASH_RESET_ACTIVE		FLASH_RESET_PORT &= 0xff ^ (1 << FLASH_RESET_BIT);
#define	SET_FLASH_RESET_INACTIVE	FLASH_RESET_PORT |= (1 << FLASH_RESET_BIT);
#define	FLASH_READY_STATE			(FLASH_BUSY_PORT & (1 << FLASH_BUSY_BIT))
// 4*125ns delay
// CAUTION: depends on clock speed
#define FLASH_500ns_DELAY	asm volatile ("nop"); asm volatile ("nop"); asm volatile ("nop"); asm volatile ("nop");


// address of the Flash Bank select register
#define	FLASH_SELECT_SECTOR(sec)		OUTB(CPLD_BASE_ADDR + FLASH_BANK_ADDR, sec)
#define	FLASH_RETURN_SECTOR				INB(CPLD_BASE_ADDR + FLASH_BANK_ADDR)
#define FLASH_SECTOR_SIZE	0x8000
#define FLASH_MAX_SECTOR	0x7F

// macros to split 32 bit address into sector and offset.
// linear address must be even!
#define	FLASH_GET_SECTOR(addr)	(((addr) >> 16) & 0xff)
#define	FLASH_GET_OFFSET(addr)	(((addr) >> 1) & 0x7fff)


// Flash QFI information
#define FLASH_QFI_QRY			0x10	// 3 byte
#define FLASH_QFI_PRI_CS		0x13	// 2 byte
#define FLASH_QFI_PRI_ADDRESS	0x15	// 2 byte
#define FLASH_QFI_VCC_MIN		0x1B
#define FLASH_QFI_VCC_MAX		0x1C
#define FLASH_QFI_T_MIN_BUF_W	0x20
#define FLASH_QFI_T_BL_ERASE	0x21
#define FLASH_QFI_DEVICE_SIZE	0x27
// PRI information offset to 0x13
#define FLASH_PRI				0x00	// 3 byte
#define FLASH_PRI_VER_MAJOR		0x03
#define FLASH_PRI_VER_MINOR		0x04

// init Flash on system startup
void init_nand_flash (void);
// write data into Flash. Returns amount of written words.
// uint8_t sector, uint16_t offset, uint16_t size, char* data
int write_nand_flash (uint8_t, uint16_t, uint16_t, uint8_t*);
// Send reset command
void reset_flash_chip (void);
// Reads the CFI Query Information
uint8_t read_flash_qfi_info (uint8_t);
// erase amount of sectors in the flash
// uint8_t amount of blocks to erase
// uint8_t first block to erase
uint8_t erase_complete_flash(uint8_t, uint8_t);
// erase a sector of Flash
// uint8_t sector
void erase_flash_sector (uint8_t);
// moves file contents to Flash memory
uint8_t file_2_nand_flash ( char *, uint32_t);
// read 16 bit value from Flash (sector, offset);
uint16_t read_flash (uint8_t, uint16_t);
// read 16 bit value from Flash (32 bit linear address);
uint16_t read_flash_abs (uint32_t);
// read 16 bit value from Flash (sector, offset) in interrupt function
uint16_t read_flash_int (uint8_t, uint16_t);

#endif // _NAND_FLASH_H_
