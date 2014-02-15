/**
 * \file MemoryMap.h
 *
 * \brief This module contains constant definitions for the memory mapping
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

#define LCD_BASE_ADDR	0x7FF0
#define CPLD_BASE_ADDR	0x7FF8


//*********************************
// CPLD Control register structure
//*********************************
// 0x0 : Mode control register
#define MODE_CTRL_ADDR 		0x00
#define		RAM_WRITE_ON_FLASH_READ		3
#define		TFT_WRITE_ON_RAM_BANK_READ	2
#define		TFT_WRITE_ON_RAM_READ		1
#define		TFT_WRITE_ON_FLASH_READ		0
// 0x1 : upper data byte register for write
#define UPPER_DATA_WR_ADDR 	0x01
// 0x2 : upper data byte register for read
#define UPPER_DATA_RD_ADDR	0x02
// 0x3 : RAM bank address low byte
#define RAM_BANK_ADDR		0x03
// 0x4 : Flash bank address low byte
#define FLASH_BANK_ADDR		0x04

//********************************
// XRAM memory usage
// 
// Banks must start at 2!
//********************************
#define XRAM_BANK_SIZE				0x2000
#define XRAM_TOC_PAGE				2
#define	XRAM_TOC_ADDR				XRAM_TOC_PAGE,0x0000
#define XRAM_PAGE_PAGE				3
#define XRAM_PAGE_ADDR				XRAM_PAGE_PAGE,0x0000
#define XRAM_GROUP_PAGE				4
#define XRAM_GROUP_ADDR				XRAM_GROUP_PAGE,0x0000
#define XRAM_OBJECT_VALUE_PAGE		5
#define XRAM_LISTEN_ELEMENTS_PAGE	6
#define XRAM_LISTEN_ELEMENTS_ADDR	XRAM_LISTEN_ELEMENTS_PAGE,0x0000
#define XRAM_CYCLIC_ELEMENTS_PAGE	7
#define XRAM_CYCLIC_ELEMENTS_ADDR	XRAM_CYCLIC_ELEMENTS_PAGE,0x0000


#define	FLASH_BASE_ADDRESS		0x8000
// 8 kB banked extra RAM area at 0x4000 to 0x5FFF
#define XRAM_BASE_ADDRESS		0x4000


#endif 
