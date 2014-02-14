/**
 * \file tft_io.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_io.h"
#ifndef __BOOTLOADER__
#include "NandFlash.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#include <avr/pgmspace.h>

int TP_X, TP_Y;
t_touch_event touch_event;
#ifndef __BOOTLOADER__
volatile uint8_t backlight_dimming;
volatile uint8_t backlight_active;
#endif
volatile uint8_t controller_type; // 0=unknown, 1 = HX8347-A, 2=SSD1289, 3=ILI9325 2.4" 180� rotated, 4=ssd1963
volatile uint16_t controller_id;
volatile uint8_t invert_touch_y; // invert X coordinate of touch position
volatile uint8_t invert_touch_x; // invert Y coordinate of touch position
volatile uint16_t screen_max_x; // max X coordinate of display
volatile uint16_t screen_max_y; // max Y coordinate of display

int lx, ly;
uint8_t current_touch_state; // 0=not touched
uint16_t sleep_timer; // timer to set display into sleep mode
uint16_t touch_timer; // timer to measure the down time
uint8_t long_down_flag; // touched for long time

uint16_t char_x, char_y; // Position of next character for system text output

//font 8*12

prog_uchar zifu[] = {
/*--  Char:     --*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*--  Char:  !  --*/
0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20, 0x00, 0x00,
/*--  Char:  "  --*/
0x00, 0x28, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*--  Char:  #  --*/
0x00, 0x00, 0x28, 0x28, 0xFC, 0x28, 0x50, 0xFC, 0x50, 0x50, 0x00, 0x00,
/*--  Char:  $  --*/
0x00, 0x20, 0x78, 0xA8, 0xA0, 0x60, 0x30, 0x28, 0xA8, 0xF0, 0x20, 0x00,
/*--  Char:  %  --*/
0x00, 0x00, 0x48, 0xA8, 0xB0, 0x50, 0x28, 0x34, 0x54, 0x48, 0x00, 0x00,
/*--  Char:  &  --*/
0x00, 0x00, 0x20, 0x50, 0x50, 0x78, 0xA8, 0xA8, 0x90, 0x6C, 0x00, 0x00,
/*--  Char:  '  --*/
0x00, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*--  Char:  (  --*/
0x00, 0x04, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x08, 0x04, 0x00,
/*--  Char:  )  --*/
0x00, 0x40, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00,
/*--  Char:  *  --*/
0x00, 0x00, 0x00, 0x20, 0xA8, 0x70, 0x70, 0xA8, 0x20, 0x00, 0x00, 0x00,
/*--  Char:  +  --*/
0x00, 0x00, 0x20, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00,
/*--  Char:  ,  --*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x80,
/*--  Char:  -  --*/
0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*--  Char:  .  --*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
/*--  Char:  /  --*/
0x00, 0x08, 0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x80, 0x00,
/*--  Char:  0  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  1  --*/
0x00, 0x00, 0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00,
/*--  Char:  2  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00, 0x00,
/*--  Char:  3  --*/
0x00, 0x00, 0x70, 0x88, 0x08, 0x30, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  4  --*/
0x00, 0x00, 0x10, 0x30, 0x50, 0x50, 0x90, 0x78, 0x10, 0x18, 0x00, 0x00,
/*--  Char:  5  --*/
0x00, 0x00, 0xF8, 0x80, 0x80, 0xF0, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  6  --*/
0x00, 0x00, 0x70, 0x90, 0x80, 0xF0, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  7  --*/
0x00, 0x00, 0xF8, 0x90, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
/*--  Char:  8  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  9  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x78, 0x08, 0x48, 0x70, 0x00, 0x00,
/*--  Char:  :  --*/
0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
/*--  Char:  ;  --*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00,
/*--  Char:  <  --*/
0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
/*--  Char:  =  --*/
0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00,
/*--  Char:  >  --*/
0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00,
/*--  Char:  ?  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x10, 0x20, 0x20, 0x00, 0x20, 0x00, 0x00,
/*--  Char:  @  --*/
0x00, 0x00, 0x70, 0x88, 0x98, 0xA8, 0xA8, 0xB8, 0x80, 0x78, 0x00, 0x00,
/*--  Char:  A  --*/
0x00, 0x00, 0x20, 0x20, 0x30, 0x50, 0x50, 0x78, 0x48, 0xCC, 0x00, 0x00,
/*--  Char:  B  --*/
0x00, 0x00, 0xF0, 0x48, 0x48, 0x70, 0x48, 0x48, 0x48, 0xF0, 0x00, 0x00,
/*--  Char:  C  --*/
0x00, 0x00, 0x78, 0x88, 0x80, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  D  --*/
0x00, 0x00, 0xF0, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0xF0, 0x00, 0x00,
/*--  Char:  E  --*/
0x00, 0x00, 0xF8, 0x48, 0x50, 0x70, 0x50, 0x40, 0x48, 0xF8, 0x00, 0x00,
/*--  Char:  F  --*/
0x00, 0x00, 0xF8, 0x48, 0x50, 0x70, 0x50, 0x40, 0x40, 0xE0, 0x00, 0x00,
/*--  Char:  G  --*/
0x00, 0x00, 0x38, 0x48, 0x80, 0x80, 0x9C, 0x88, 0x48, 0x30, 0x00, 0x00,
/*--  Char:  H  --*/
0x00, 0x00, 0xCC, 0x48, 0x48, 0x78, 0x48, 0x48, 0x48, 0xCC, 0x00, 0x00,
/*--  Char:  I  --*/
0x00, 0x00, 0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xF8, 0x00, 0x00,
/*--  Char:  J  --*/
0x00, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0xE0, 0x00,
/*--  Char:  K  --*/
0x00, 0x00, 0xEC, 0x48, 0x50, 0x60, 0x50, 0x50, 0x48, 0xEC, 0x00, 0x00,
/*--  Char:  L  --*/
0x00, 0x00, 0xE0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x44, 0xFC, 0x00, 0x00,
/*--  Char:  M  --*/
0x00, 0x00, 0xD8, 0xD8, 0xD8, 0xD8, 0xA8, 0xA8, 0xA8, 0xA8, 0x00, 0x00,
/*--  Char:  N  --*/
0x00, 0x00, 0xDC, 0x48, 0x68, 0x68, 0x58, 0x58, 0x48, 0xE8, 0x00, 0x00,
/*--  Char:  O  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00,
/*--  Char:  P  --*/
0x00, 0x00, 0xF0, 0x48, 0x48, 0x70, 0x40, 0x40, 0x40, 0xE0, 0x00, 0x00,
/*--  Char:  Q  --*/
0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0xE8, 0x98, 0x70, 0x18, 0x00,
/*--  Char:  R  --*/
0x00, 0x00, 0xF0, 0x48, 0x48, 0x70, 0x50, 0x48, 0x48, 0xEC, 0x00, 0x00,
/*--  Char:  S  --*/
0x00, 0x00, 0x78, 0x88, 0x80, 0x60, 0x10, 0x08, 0x88, 0xF0, 0x00, 0x00,
/*--  Char:  T  --*/
0x00, 0x00, 0xF8, 0xA8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00,
/*--  Char:  U  --*/
0x00, 0x00, 0xCC, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x30, 0x00, 0x00,
/*--  Char:  V  --*/
0x00, 0x00, 0xCC, 0x48, 0x48, 0x50, 0x50, 0x30, 0x20, 0x20, 0x00, 0x00,
/*--  Char:  W  --*/
0x00, 0x00, 0xA8, 0xA8, 0xA8, 0x70, 0x50, 0x50, 0x50, 0x50, 0x00, 0x00,
/*--  Char:  X  --*/
0x00, 0x00, 0xD8, 0x50, 0x50, 0x20, 0x20, 0x50, 0x50, 0xD8, 0x00, 0x00,
/*--  Char:  Y  --*/
0x00, 0x00, 0xD8, 0x50, 0x50, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00,
/*--  Char:  Z  --*/
0x00, 0x00, 0xF8, 0x90, 0x10, 0x20, 0x20, 0x40, 0x48, 0xF8, 0x00, 0x00,
#ifndef __BOOTLOADER__
		/*--  Char:  [  --*/
		0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00,
		/*--  Char:  \  --*/
		0x00, 0x40, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10, 0x08, 0x00, 0x00,
		/*--  Char:  ]  --*/
		0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x70, 0x00,
		/*--  Char:  ^  --*/
		0x00, 0x20, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/*--  Char:  _  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
		/*--  Char:  `  --*/
		0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/*--  Char:  a  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x38, 0x48, 0x3C, 0x00, 0x00,
		/*--  Char:  b  --*/
		0x00, 0x00, 0xC0, 0x40, 0x40, 0x70, 0x48, 0x48, 0x48, 0x70, 0x00, 0x00,
		/*--  Char:  c  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x48, 0x40, 0x40, 0x38, 0x00, 0x00,
		/*--  Char:  d  --*/
		0x00, 0x00, 0x18, 0x08, 0x08, 0x38, 0x48, 0x48, 0x48, 0x3C, 0x00, 0x00,
		/*--  Char:  e  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x78, 0x40, 0x38, 0x00, 0x00,
		/*--  Char:  f  --*/
		0x00, 0x00, 0x1C, 0x20, 0x20, 0x78, 0x20, 0x20, 0x20, 0x78, 0x00, 0x00,
		/*--  Char:  g  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x48, 0x30, 0x40, 0x78, 0x44, 0x38,
		/*--  Char:  h  --*/
		0x00, 0x00, 0xC0, 0x40, 0x40, 0x70, 0x48, 0x48, 0x48, 0xEC, 0x00, 0x00,
		/*--  Char:  i  --*/
		0x00, 0x00, 0x20, 0x00, 0x00, 0x60, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00,
		/*--  Char:  j  --*/
		0x00, 0x00, 0x10, 0x00, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0xE0,
		/*--  Char:  k  --*/
		0x00, 0x00, 0xC0, 0x40, 0x40, 0x5C, 0x50, 0x70, 0x48, 0xEC, 0x00, 0x00,
		/*--  Char:  l  --*/
		0x00, 0x00, 0xE0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xF8, 0x00, 0x00,
		/*--  Char:  m  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xA8, 0xA8, 0xA8, 0xA8, 0x00, 0x00,
		/*--  Char:  n  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x48, 0x48, 0x48, 0xEC, 0x00, 0x00,
		/*--  Char:  o  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x48, 0x48, 0x30, 0x00, 0x00,
		/*--  Char:  p  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x48, 0x48, 0x48, 0x70, 0x40, 0xE0,
		/*--  Char:  q  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x48, 0x48, 0x48, 0x38, 0x08, 0x1C,
		/*--  Char:  r  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x60, 0x40, 0x40, 0xE0, 0x00, 0x00,
		/*--  Char:  s  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x40, 0x30, 0x08, 0x78, 0x00, 0x00,
		/*--  Char:  t  --*/
		0x00, 0x00, 0x00, 0x20, 0x20, 0x70, 0x20, 0x20, 0x20, 0x18, 0x00, 0x00,
		/*--  Char:  u  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x48, 0x48, 0x48, 0x3C, 0x00, 0x00,
		/*--  Char:  v  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x48, 0x50, 0x30, 0x20, 0x00, 0x00,
		/*--  Char:  w  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0xA8, 0x70, 0x50, 0x50, 0x00, 0x00,
		/*--  Char:  x  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x50, 0x20, 0x50, 0xD8, 0x00, 0x00,
		/*--  Char:  y  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x48, 0x50, 0x30, 0x20, 0x20, 0xC0,
		/*--  Char:  z  --*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x10, 0x20, 0x20, 0x78, 0x00, 0x00,
		/*--  Char:  {  --*/
		0x00, 0x18, 0x10, 0x10, 0x10, 0x20, 0x10, 0x10, 0x10, 0x10, 0x18, 0x00,
		/*--  Char:  |  --*/
		0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
		/*--  Char:  }  --*/
		0x00, 0x60, 0x20, 0x20, 0x20, 0x10, 0x20, 0x20, 0x20, 0x20, 0x60, 0x00,
		/*--  Char:  ~  --*/
		0x40, 0xA4, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#endif
		};

/* returns the contents of the cpld control register or the tft data output
 * a=0: tft data output
 * a=1: control register
 */
uint8_t read_cpld(uint8_t a) {

	return INB ( LCD_BASE_ADDR + a);

}

/* writes a byte into a cpld register.
 * a=0: data register
 * a=1: control register
 */
void write_cpld(uint8_t d, uint8_t a) {

	OUTB( LCD_BASE_ADDR + a, d);

}

/* This function writes to the tft pointer register
 */
void tft_set_pointer(uint8_t ptr) {

	// write low byte to LCD pointer register
	OUTB( LCD_BASE_ADDR + LCD_POINTER, ptr);
}

/* This function writes a byte to the tft data register
 */
void tft_write_byte(uint8_t d) {

	// write low byte to LCD pointer register
	OUTB( LCD_BASE_ADDR + LCD_DATA, d);

}

/* This function reads a byte from the tft data register
 */
uint8_t tft_read_byte(void) {

	// read low byte from LCD data register
	return INB ( LCD_BASE_ADDR + LCD_DATA );

}

/* This function writes a word to the tft data register
 */
void tft_write_word(uint16_t d) {

	// write high byte to CPLD data register
	OUTB( CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, d >> 8);
	// write low byte to LCD
	OUTB( LCD_BASE_ADDR + LCD_DATA, d & 0xff);

}

/* This function writes a word to the tft data register
 */
uint16_t tft_read_word(void) {

	uint8_t lb;
	uint16_t hb;

	// read low byte from LCD controller
	lb = INB ( LCD_BASE_ADDR + LCD_DATA );
	// read high byte from CPLD
	hb = INB ( CPLD_BASE_ADDR + UPPER_DATA_RD_ADDR );

	return (hb << 8) | lb;

}

// activates the reset of the tft-lcd
void tft_set_reset_active(void) {
	TFT_SET_RESET_ACTIVE
}

// inactivates the reset of the tft-lcd
void tft_set_reset_inactive(void) {
	TFT_SET_RESET_INACTIVE
}

/* controls the ft backlight
 * light = 0: off
 * light = 1: on
 */
void tft_backlight(uint8_t light) {
	if (light)
		TFT_BACKLIGHT_ON
	else
		TFT_BACKLIGHT_OFF
}

void main_W_com_data(uint8_t com1, uint16_t dat1) {
	tft_set_pointer(com1);
	tft_write_word(dat1);
}

uint16_t main_W_com_read_data(uint8_t com1) {

	tft_set_pointer(com1);
	return tft_read_word();
}

uint8_t main_com_read_data(uint8_t com1) {

	tft_set_pointer(com1);
	return tft_read_byte();
}

/* sents the init sequence to the tft controller
 */
void tft_init_sequence(void) {

	controller_type = CTRL_UNKNOWN;

#ifndef __BOOTLOADER__
	while (controller_type == CTRL_UNKNOWN) {
#endif
		tft_set_reset_inactive();
		NutDelay(6);
		tft_set_reset_active();
		NutDelay(10);
		tft_set_reset_inactive();
		NutDelay(20);

		// autodetect sequence:
		controller_id = main_W_com_read_data(0);
		// get R00
#ifndef __BOOTLOADER__
#ifdef LCD_DEBUG
		printf_P(PSTR("\nR00=%4.4x\n"), controller_id);
#endif
#endif
		if ((controller_id & HX8347_MASK) == HX8347_R00) {

#ifndef __BOOTLOADER__

			// try  SSD1963
			NutDelay(100);

			volatile uint8_t b0, b1, b2, b3, b4;

			tft_set_pointer(SSD1963_read_ddb);
			NutDelay(10);
			b0 = tft_read_byte();
			b1 = tft_read_byte();
			b2 = tft_read_byte();
			b3 = tft_read_byte();
			b4 = tft_read_byte();

#ifdef LCD_DEBUG
			printf_P(PSTR("\nSSD1963: %2.2x %2.2x %2.2x %2.2x %2.2x\n"), b0, b1, b2, b3, b4);
			printf_P(PSTR("SSD1963: %2.2x %2.2x %2.2x %2.2x %2.2x\n"), SSD1963_SSL_H, SSD1963_SSL_L, SSD1963_PROD, SSD1963_REV, SSD1963_EXIT);
#endif

			/*
			 SSL[15:8] : Supplier ID of Solomon Systech Limited high byte, always 01h (POR = 00000001)
			 SSL[7:0] : Supplier ID of Solomon Systech Limited low byte, always 57h (POR = 010101110)
			 PROD[7:0] : Product ID, always 61h (POR = 01100001)
			 REV[2:0] : Revision code, always 01h (POR = 001)
			 Exit code, always FFh (POR = 11111111)
			 */
			if ((b0 == SSD1963_SSL_H) && (b1 == SSD1963_SSL_L)
					&& (b2 == SSD1963_PROD) && (b3 == SSD1963_REV)
					&& (b4 == SSD1963_EXIT)) {
				controller_type = CTRL_SSD1963;
			} else
#endif // #ifndef __BOOTLOADER__
				controller_type = CTRL_HX8347;
		}
#ifndef __BOOTLOADER__
		else if ((controller_id & ILI9325_MASK) == ILI9325_R00) {
			controller_type = CTRL_ILI9325;
		}
#endif
		else if ((controller_id & SSD1289_MASK) == SSD1289_R00) {
			controller_type = CTRL_SSD1289;
		}

#ifndef __BOOTLOADER__

	}
#ifdef LCD_DEBUG
	printf_P(PSTR("ID=%d\n"), controller_type);
#endif
#endif

	if (controller_type == CTRL_HX8347) {
		main_W_com_data(0x46, 0x00A4);
		main_W_com_data(0x47, 0x0053);
		main_W_com_data(0x48, 0x0000);
		main_W_com_data(0x49, 0x0044);
		main_W_com_data(0x4a, 0x0004);
		main_W_com_data(0x4b, 0x0067);
		main_W_com_data(0x4c, 0x0033);
		main_W_com_data(0x4d, 0x0077);
		main_W_com_data(0x4e, 0x0012);
		main_W_com_data(0x4f, 0x004C);
		main_W_com_data(0x50, 0x0046);
		main_W_com_data(0x51, 0x0044);

		//240x320 window setting
		main_W_com_data(0x02, 0x0000); // Column address start2
		main_W_com_data(0x03, 0x0000); // Column address start1
		main_W_com_data(0x04, 0x0000); // Column address end2
		main_W_com_data(0x05, 0x00ef); // Column address end1
		main_W_com_data(0x06, 0x0000); // Row address start2
		main_W_com_data(0x07, 0x0000); // Row address start1
		main_W_com_data(0x08, 0x0001); // Row address end2
		main_W_com_data(0x09, 0x003f); // Row address end1

		// Display Setting
		main_W_com_data(0x01, 0x0006); // IDMON=0, INVON=1, NORON=1, PTLON=0
		main_W_com_data(0x16, 0x0068); // MY=0, MX=1, MV=1, ML=1, BGR=0, TEON=0   0048
		main_W_com_data(0x23, 0x0095); // N_DC=1001 0101
		main_W_com_data(0x24, 0x0095); // PI_DC=1001 0101
		main_W_com_data(0x25, 0x00FF); // I_DC=1111 1111

		main_W_com_data(0x27, 0x0002); // N_BP=0000 0010
		main_W_com_data(0x28, 0x0002); // N_FP=0000 0010
		main_W_com_data(0x29, 0x0002); // PI_BP=0000 0010
		main_W_com_data(0x2a, 0x0002); // PI_FP=0000 0010
		main_W_com_data(0x2C, 0x0002); // I_BP=0000 0010
		main_W_com_data(0x2d, 0x0002); // I_FP=0000 0010

		main_W_com_data(0x3a, 0x0001); // N_RTN=0000, N_NW=001    0001
		main_W_com_data(0x3b, 0x0000); // P_RTN=0000, P_NW=001
		main_W_com_data(0x3c, 0x00f0); // I_RTN=1111, I_NW=000
		main_W_com_data(0x3d, 0x0000); // DIV=00
		NutDelay(2);
		main_W_com_data(0x35, 0x0038); // EQS=38h
		main_W_com_data(0x36, 0x0078); // EQP=78h
		main_W_com_data(0x3E, 0x0038); // SON=38h
		main_W_com_data(0x40, 0x000F); // GDON=0Fh
		main_W_com_data(0x41, 0x00F0); // GDOFF

		// Power Supply Setting
		main_W_com_data(0x19, 0x0049); // CADJ=0100, CUADJ=100, OSD_EN=1 ,60Hz
		main_W_com_data(0x93, 0x000F); // RADJ=1111, 100%
		NutDelay(2);
		main_W_com_data(0x20, 0x0040); // BT=0100
		main_W_com_data(0x1D, 0x0007); // VC1=111   0007
		main_W_com_data(0x1E, 0x0000); // VC3=000
		main_W_com_data(0x1F, 0x0004); // VRH=0011

		//VCOM SETTING
		main_W_com_data(0x44, 0x004D); // VCM=101 0000  4D
		main_W_com_data(0x45, 0x000E); // VDV=1 0001   0011
		NutDelay(2);
		main_W_com_data(0x1C, 0x0004); // AP=100
		NutDelay(3);

		main_W_com_data(0x1B, 0x0018); // GASENB=0, PON=0, DK=1, XDK=0, VLCD_TRI=0, STB=0
		NutDelay(2);
		main_W_com_data(0x1B, 0x0010); // GASENB=0, PON=1, DK=0, XDK=0, VLCD_TRI=0, STB=0
		NutDelay(2);
		main_W_com_data(0x43, 0x0080); //set VCOMG=1
		NutDelay(3);

		// Display ON Setting
		main_W_com_data(0x90, 0x007F); // SAP=0111 1111
		main_W_com_data(0x26, 0x0004); //GON=0, DTE=0, D=01
		NutDelay(2);
		main_W_com_data(0x26, 0x0024); //GON=1, DTE=0, D=01
		main_W_com_data(0x26, 0x002C); //GON=1, DTE=0, D=11
		NutDelay(2);
		main_W_com_data(0x26, 0x003C); //GON=1, DTE=1, D=11

		// INTERNAL REGISTER SETTING
		main_W_com_data(0x57, 0x0002); // TEST_Mode=1: into TEST mode
		main_W_com_data(0x95, 0x0001); // SET DISPLAY CLOCK AND PUMPING CLOCK TO SYNCHRONIZE
		main_W_com_data(0x57, 0x0000); // TEST_Mode=0: exit TEST mode
	}
#ifndef __BOOTLOADER__
	else if (controller_type == CTRL_ILI9325) {

#define TS_VAL_ENTRY_MOD			0x1038
#define TS_INS_START_OSC			0x00 //data read at this instruction should be 0x0789 --> use for test connection
#define TS_INS_DRIV_OUT_CTRL		0x01
#define TS_INS_DRIV_WAV_CTRL		0x02
#define TS_INS_ENTRY_MOD			0x03
#define TS_INS_RESIZE_CTRL			0x04
#define TS_INS_DISP_CTRL1			0x07
#define TS_INS_DISP_CTRL2			0x08
#define TS_INS_DISP_CTRL3			0x09
#define TS_INS_DISP_CTRL4			0x0A
#define TS_INS_RGB_DISP_IF_CTRL1	0x0C
#define TS_INS_FRM_MARKER_POS		0x0D
#define TS_INS_RGB_DISP_IF_CTRL2	0x0F
#define TS_INS_POW_CTRL1			0x10
#define TS_INS_POW_CTRL2			0x11
#define TS_INS_POW_CTRL3			0x12
#define TS_INS_POW_CTRL4			0x13
#define TS_INS_GRAM_HOR_AD			0x20
#define TS_INS_GRAM_VER_AD			0x21
#define TS_INS_RW_GRAM				0x22
#define TS_INS_POW_CTRL7			0x29
#define TS_INS_FRM_RATE_COL_CTRL	0x2B
#define TS_INS_GAMMA_CTRL1			0x30
#define TS_INS_GAMMA_CTRL2			0x31
#define TS_INS_GAMMA_CTRL3			0x32
#define TS_INS_GAMMA_CTRL4			0x35
#define TS_INS_GAMMA_CTRL5			0x36
#define TS_INS_GAMMA_CTRL6			0x37
#define TS_INS_GAMMA_CTRL7			0x38
#define TS_INS_GAMMA_CTRL8			0x39
#define TS_INS_GAMMA_CTRL9			0x3C
#define TS_INS_GAMMA_CTRL10			0x3D
#define TS_INS_HOR_START_AD			0x50
#define TS_INS_HOR_END_AD			0x51
#define TS_INS_VER_START_AD			0x52
#define TS_INS_VER_END_AD			0x53
#define TS_INS_GATE_SCAN_CTRL1		0x60
#define TS_INS_GATE_SCAN_CTRL2		0x61
#define TS_INS_GATE_SCAN_CTRL3		0x6A
#define TS_INS_PART_IMG1_DISP_POS	0x80
#define TS_INS_PART_IMG1_START_AD	0x81
#define TS_INS_PART_IMG1_END_AD		0x82
#define TS_INS_PART_IMG2_DISP_POS	0x83
#define TS_INS_PART_IMG2_START_AD	0x84
#define TS_INS_PART_IMG2_END_AD		0x85
#define TS_INS_PANEL_IF_CTRL1		0x90
#define TS_INS_PANEL_IF_CTRL2		0x92
#define TS_INS_PANEL_IF_CTRL3		0x93
#define TS_INS_PANEL_IF_CTRL4		0x95
#define TS_INS_PANEL_IF_CTRL5		0x97
#define TS_INS_PANEL_IF_CTRL6		0x98

		main_W_com_data(0xE3, 0x3008);
		main_W_com_data(0xE7, 0x0012);
		main_W_com_data(0xEF, 0x1231);

		main_W_com_data(TS_INS_START_OSC, 0x0001); //start oscillator
		NutDelay(50);

		main_W_com_data(TS_INS_DRIV_OUT_CTRL, 0x0000); //set SS, SM
		main_W_com_data(TS_INS_DRIV_WAV_CTRL, 0x0700); //set 1 line inversion

		main_W_com_data(TS_INS_ENTRY_MOD, TS_VAL_ENTRY_MOD); //set GRAM write direction, BGR=0

		main_W_com_data(TS_INS_RESIZE_CTRL, 0x0000); //no resizing

		main_W_com_data(TS_INS_DISP_CTRL2, 0x0202); //front & back porch periods = 2
		main_W_com_data(TS_INS_DISP_CTRL3, 0x0000);
		main_W_com_data(TS_INS_DISP_CTRL4, 0x0000);

		main_W_com_data(TS_INS_RGB_DISP_IF_CTRL1, 0x0000); //select system interface

		main_W_com_data(TS_INS_FRM_MARKER_POS, 0x0000);
		main_W_com_data(TS_INS_RGB_DISP_IF_CTRL2, 0x0000);

		main_W_com_data(TS_INS_POW_CTRL1, 0x0000);
		main_W_com_data(TS_INS_POW_CTRL2, 0x0007);
		main_W_com_data(TS_INS_POW_CTRL3, 0x0000);
		main_W_com_data(TS_INS_POW_CTRL4, 0x0000);

		NutDelay(200);

		main_W_com_data(TS_INS_POW_CTRL1, 0x1690);
		main_W_com_data(TS_INS_POW_CTRL2, 0x0227);

		NutDelay(50);

		main_W_com_data(TS_INS_POW_CTRL3, 0x001A);

		NutDelay(50);

		main_W_com_data(TS_INS_POW_CTRL4, 0x1800);
		main_W_com_data(TS_INS_POW_CTRL7, 0x002A);

		NutDelay(50);

		main_W_com_data(TS_INS_GRAM_HOR_AD, 0x0000);
		main_W_com_data(TS_INS_GRAM_VER_AD, 0x0000);

		main_W_com_data(TS_INS_GAMMA_CTRL1, 0x0007);
		main_W_com_data(TS_INS_GAMMA_CTRL2, 0x0605);
		main_W_com_data(TS_INS_GAMMA_CTRL3, 0x0106);
		main_W_com_data(TS_INS_GAMMA_CTRL4, 0x0206);
		main_W_com_data(TS_INS_GAMMA_CTRL5, 0x0808);
		main_W_com_data(TS_INS_GAMMA_CTRL6, 0x0007);
		main_W_com_data(TS_INS_GAMMA_CTRL7, 0x0201);
		main_W_com_data(TS_INS_GAMMA_CTRL8, 0x0007);
		main_W_com_data(TS_INS_GAMMA_CTRL9, 0x0602);
		main_W_com_data(TS_INS_GAMMA_CTRL10, 0x0808);

		main_W_com_data(TS_INS_HOR_START_AD, 0x0000);
		main_W_com_data(TS_INS_HOR_END_AD, 0x00EF);
		main_W_com_data(TS_INS_VER_START_AD, 0x0000);
		main_W_com_data(TS_INS_VER_END_AD, 0x013F);
		main_W_com_data(TS_INS_GATE_SCAN_CTRL1, 0xA700);
		main_W_com_data(TS_INS_GATE_SCAN_CTRL2, 0x0001);
		main_W_com_data(TS_INS_GATE_SCAN_CTRL3, 0x0000);

		main_W_com_data(TS_INS_PART_IMG1_DISP_POS, 0x0000);
		main_W_com_data(TS_INS_PART_IMG1_START_AD, 0x0000);
		main_W_com_data(TS_INS_PART_IMG1_END_AD, 0x0000);
		main_W_com_data(TS_INS_PART_IMG2_DISP_POS, 0x0000);
		main_W_com_data(TS_INS_PART_IMG2_START_AD, 0x0000);
		main_W_com_data(TS_INS_PART_IMG2_END_AD, 0x0000);

		main_W_com_data(TS_INS_PANEL_IF_CTRL1, 0x0010);
		main_W_com_data(TS_INS_PANEL_IF_CTRL2, 0x0000);
		main_W_com_data(TS_INS_PANEL_IF_CTRL3, 0x0003);
		main_W_com_data(TS_INS_PANEL_IF_CTRL4, 0x0110);
		main_W_com_data(TS_INS_PANEL_IF_CTRL5, 0x0000);
		main_W_com_data(TS_INS_PANEL_IF_CTRL6, 0x0000);

		main_W_com_data(TS_INS_DISP_CTRL1, 0x0133);
	}

	else if (controller_type == CTRL_SSD1963) {

#ifdef TFT_480_800_CTRL_SSD1963

#define		HDP		799
#define		HT		928
#define		HPS		46
#define 	LPS		15
#define 	HPW		48

#define		VDP		479
#define 	VT		525
#define		VPS		16
#define		FPS		8
#define		VPW		16

		// enable wait
		XMCRA |= (1<<SRW00) | (1<<SRW01);// wait

		tft_set_pointer(SSD1963_set_pll_mn);//PLL multiplier, set PLL clock to 120M
		tft_write_byte(0x0023);//N=0x36 for 6.5M, 0x23 for 10M crystal
		tft_write_byte(0x0002);
		tft_write_byte(0x0004);
		tft_set_pointer(SSD1963_set_pll);// PLL enable
		tft_write_byte(0x0001);
		NutDelay(1);
		tft_set_pointer(SSD1963_set_pll);
		tft_write_byte(0x0003);
		NutDelay(5);
		tft_set_pointer(SSD1963_soft_reset);// software reset
		NutDelay(5);

		// disable wait
		XMCRA &= ~((1<<SRW00) | (1<<SRW01));// wait

		tft_set_pointer(SSD1963_set_lshift_freq);//PLL setting for PCLK, depends on resolution
		tft_write_byte(0x0003);
		tft_write_byte(0x00ff);
		tft_write_byte(0x00ff);

		tft_set_pointer(SSD1963_set_lcd_mode);//LCD SPECIFICATION
		tft_write_byte(0x0027);
		tft_write_byte(0x0000);
		tft_write_byte((HDP>>8)&0X00FF);//Set HDP
		tft_write_byte(HDP&0X00FF);
		tft_write_byte((VDP>>8)&0X00FF);//Set VDP
		tft_write_byte(VDP&0X00FF);
		tft_write_byte(0x0000);

		tft_set_pointer(SSD1963_set_hori_period);//HSYNC
		tft_write_byte((HT>>8)&0X00FF);//Set HT
		tft_write_byte(HT&0X00FF);
		tft_write_byte((HPS>>8)&0X00FF);//Set HPS
		tft_write_byte(HPS&0X00FF);
		tft_write_byte(HPW);//Set HPW
		tft_write_byte((LPS>>8)&0X00FF);//Set HPS
		tft_write_byte(LPS&0X00FF);
		tft_write_byte(0x0000);

		tft_set_pointer(SSD1963_set_vert_period);//VSYNC
		tft_write_byte((VT>>8)&0X00FF);//Set VT
		tft_write_byte(VT&0X00FF);
		tft_write_byte((VPS>>8)&0X00FF);//Set VPS
		tft_write_byte(VPS&0X00FF);
		tft_write_byte(VPW);//Set VPW
		tft_write_byte((FPS>>8)&0X00FF);//Set FPS
		tft_write_byte(FPS&0X00FF);

		tft_set_pointer(SSD1963_set_gpio_value);
		tft_write_byte(0x000F);//GPIO[3:0] out 1

		tft_set_pointer(SSD1963_set_gpio_conf);
		tft_write_byte(0x0007);//GPIO3=input, GPIO[2:0]=output
		tft_write_byte(0x0001);//GPIO0 normal

		tft_set_pointer(SSD1963_set_address_mode);//rotation
		tft_write_byte(0x0000);

		tft_set_pointer(SSD1963_set_pixel_data_interface);//pixel data interface
		tft_write_byte(0x0003);

#ifdef LCD_DEBUG
		tft_set_pointer(SSD1963_get_pixel_data_interface); //pixel data interface
		printf_P(PSTR("\nint: %2.2x\n"), tft_read_byte());
#endif
		NutDelay(5);

		tft_set_pointer(SSD1963_set_display_on); //display on

		tft_set_pointer(SSD1963_set_dbc_conf);
		tft_write_byte(0x000d);
	}

#else

// Select 4.3" LCD
#ifdef TFT_272_480_CTRL_SSD1963_NEW

// 480 x 272 pixel display	with SD Slot
#define		HDP		479
#define		HT		531
#define		HPS		43
#define 	LPS		8
#define 	HPW		10

#define		VDP		271
#define 	VT		288
#define		VPS		12
#define		FPS		4
#define		VPW		10

		// enable wait
		XMCRA |= (1 << SRW00) | (1 << SRW01); // wait

		tft_set_pointer(SSD1963_set_pll_mn); //PLL multiplier, set PLL clock to 120M
		tft_write_byte(0x002d); //N=0x36 for 6.5M, 0x23 for 10M crystal
		tft_write_byte(0x0002);
		tft_write_byte(0x0004);
		tft_set_pointer(SSD1963_set_pll); // PLL enable
		tft_write_byte(0x0001);
		NutDelay(1);
		tft_set_pointer(SSD1963_set_pll);
		tft_write_byte(0x0003);
		NutDelay(5);
		tft_set_pointer(SSD1963_soft_reset); // software reset
		NutDelay(5);

		// disable wait
		XMCRA &= ~((1 << SRW00) | (1 << SRW01)); // wait

		tft_set_pointer(SSD1963_set_lshift_freq); //PLL setting for PCLK, depends on resolution
		tft_write_byte(0x0000);
		tft_write_byte(0x00ff);
		tft_write_byte(0x00be);

		tft_set_pointer(SSD1963_set_lcd_mode); //LCD SPECIFICATION
		tft_write_byte(0x0020);
		tft_write_byte(0x0000);
		tft_write_byte((HDP >> 8) & 0X00FF); //Set HDP
		tft_write_byte(HDP & 0X00FF);
		tft_write_byte((VDP >> 8) & 0X00FF); //Set VDP
		tft_write_byte(VDP & 0X00FF);
		tft_write_byte(0x0000);
		NutDelay(5);

		tft_set_pointer(SSD1963_set_hori_period); //HSYNC
		tft_write_byte((HT >> 8) & 0X00FF); //Set HT
		tft_write_byte(HT & 0X00FF);
		tft_write_byte((HPS >> 8) & 0X00FF); //Set HPS
		tft_write_byte(HPS & 0X00FF);
		tft_write_byte(HPW); //Set HPW
		tft_write_byte((LPS >> 8) & 0X00FF); //Set HPS
		tft_write_byte(LPS & 0X00FF);
		tft_write_byte(0x0000);

		tft_set_pointer(SSD1963_set_vert_period); //VSYNC
		tft_write_byte((VT >> 8) & 0X00FF); //Set VT
		tft_write_byte(VT & 0X00FF);
		tft_write_byte((VPS >> 8) & 0X00FF); //Set VPS
		tft_write_byte(VPS & 0X00FF);
		tft_write_byte(VPW); //Set VPW
		tft_write_byte((FPS >> 8) & 0X00FF); //Set FPS
		tft_write_byte(FPS & 0X00FF);

		tft_set_pointer(SSD1963_set_gpio_value);
		tft_write_byte(0x0000); //GPIO[3:0] out 1

		tft_set_pointer(SSD1963_set_gpio_conf);
		tft_write_byte(0x0000); //GPIO3=input, GPIO[2:0]=output
		tft_write_byte(0x0001); //GPIO0 normal

		tft_set_pointer(SSD1963_set_address_mode); //rotation
		tft_write_byte(0x0003);

		tft_set_pointer(SSD1963_set_pixel_data_interface); //pixel data interface
		tft_write_byte(0x0003);

		NutDelay(5);

		tft_set_pointer(SSD1963_set_display_on); //display on

		tft_set_pointer(SSD1963_set_dbc_conf);
		tft_write_byte(0x000d);
	}
#else

// 480 x 272 pixel display	without SD Slot
#define		HDP		479
#define		HT		531
#define		HPS		43
#define 	LPS		8
#define 	HPW		1

#define		VDP		271
#define 	VT		288
#define		VPS		12
#define		FPS		4
#define		VPW		10

		// enable wait
		XMCRA |= (1 << SRW00) | (1 << SRW01); // wait

		tft_set_pointer(SSD1963_set_pll_mn); //PLL multiplier, set PLL clock to 120M
		tft_write_byte(0x0023); //N=0x36 for 6.5M, 0x23 for 10M crystal
		tft_write_byte(0x0002);
		tft_write_byte(0x0004);
		tft_set_pointer(SSD1963_set_pll); // PLL enable
		tft_write_byte(0x0001);
		NutDelay(1);
		tft_set_pointer(SSD1963_set_pll);
		tft_write_byte(0x0003);
		NutDelay(5);
		tft_set_pointer(SSD1963_soft_reset); // software reset
		NutDelay(5);

		// disable wait
		XMCRA &= ~((1 << SRW00) | (1 << SRW01)); // wait

		tft_set_pointer(SSD1963_set_lshift_freq); //PLL setting for PCLK, depends on resolution
		tft_write_byte(0x0003);
		tft_write_byte(0x0033);
		tft_write_byte(0x0032);

		tft_set_pointer(SSD1963_set_lcd_mode); //LCD SPECIFICATION
		tft_write_byte(0x0000);
		tft_write_byte(0x0000);
		tft_write_byte((HDP >> 8) & 0X00FF); //Set HDP
		tft_write_byte(HDP & 0X00FF);
		tft_write_byte((VDP >> 8) & 0X00FF); //Set VDP
		tft_write_byte(VDP & 0X00FF);
		tft_write_byte(0x002D);

		tft_set_pointer(SSD1963_set_hori_period); //HSYNC
		tft_write_byte((HT >> 8) & 0X00FF); //Set HT
		tft_write_byte(HT & 0X00FF);
		tft_write_byte((HPS >> 8) & 0X00FF); //Set HPS
		tft_write_byte(HPS & 0X00FF);
		tft_write_byte(HPW); //Set HPW
		tft_write_byte((LPS >> 8) & 0X00FF); //Set HPS
		tft_write_byte(LPS & 0X00FF);
		tft_write_byte(0x0000);

		tft_set_pointer(SSD1963_set_vert_period); //VSYNC
		tft_write_byte((VT >> 8) & 0X00FF); //Set VT
		tft_write_byte(VT & 0X00FF);
		tft_write_byte((VPS >> 8) & 0X00FF); //Set VPS
		tft_write_byte(VPS & 0X00FF);
		tft_write_byte(VPW); //Set VPW
		tft_write_byte((FPS >> 8) & 0X00FF); //Set FPS
		tft_write_byte(FPS & 0X00FF);

		tft_set_pointer(SSD1963_set_gpio_value);
		tft_write_byte(0x000F); //GPIO[3:0] out 1

		tft_set_pointer(SSD1963_set_gpio_conf);
		tft_write_byte(0x0007); //GPIO3=input, GPIO[2:0]=output
		tft_write_byte(0x0001); //GPIO0 normal

		tft_set_pointer(SSD1963_set_address_mode); //rotation
		tft_write_byte(0x0003);

		tft_set_pointer(SSD1963_set_pixel_data_interface); //pixel data interface
		tft_write_byte(0x0003);

		NutDelay(5);

		tft_set_pointer(SSD1963_set_display_on); //display on

		tft_set_pointer(SSD1963_set_dbc_conf);
		tft_write_byte(0x000d);
	}
#endif // TFT_272_480_CTRL_SSD1963_NEW
#endif // TFT_480_800_CTRL_SSD1963
#endif // __BOOTLOADER__
	else if (controller_type == CTRL_SSD1289) {

		main_W_com_data(0x0000, 0x0001);
		NutDelay(2);
		main_W_com_data(0x0003, 0xA8A4);
		NutDelay(2);
		main_W_com_data(0x000C, 0x0000);
		NutDelay(2);
		main_W_com_data(0x000D, 0x080C);
		NutDelay(2);
		main_W_com_data(0x000E, 0x2B00);
		NutDelay(2);
		main_W_com_data(0x001E, 0x00B0);
		NutDelay(2);
		main_W_com_data(0x0001, 0x293F);
		NutDelay(2); //320*240  0x6B3F
		main_W_com_data(0x0002, 0x0600);
		NutDelay(2);
		main_W_com_data(0x0010, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0011, 0x6078);
		NutDelay(2); //0x4030
		main_W_com_data(0x0005, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0006, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0016, 0xEF1C);
		NutDelay(2);
		main_W_com_data(0x0017, 0x0003);
		NutDelay(2);
		main_W_com_data(0x0007, 0x0233);
		NutDelay(2);
		main_W_com_data(0x000B, 0x0000);
		NutDelay(2);
		main_W_com_data(0x000F, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0041, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0042, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0048, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0049, 0x013F);
		NutDelay(2);
		main_W_com_data(0x004A, 0x0000);
		NutDelay(2);
		main_W_com_data(0x004B, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0044, 0xEF00);
		NutDelay(2);
		main_W_com_data(0x0045, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0046, 0x013F);
		NutDelay(2);
		main_W_com_data(0x0030, 0x0707);
		NutDelay(2);
		main_W_com_data(0x0031, 0x0204);
		NutDelay(2);
		main_W_com_data(0x0032, 0x0204);
		NutDelay(2);
		main_W_com_data(0x0033, 0x0502);
		NutDelay(2);
		main_W_com_data(0x0034, 0x0507);
		NutDelay(2);
		main_W_com_data(0x0035, 0x0204);
		NutDelay(2);
		main_W_com_data(0x0036, 0x0204);
		NutDelay(2);
		main_W_com_data(0x0037, 0x0502);
		NutDelay(2);
		main_W_com_data(0x003A, 0x0302);
		NutDelay(2);
		main_W_com_data(0x003B, 0x0302);
		NutDelay(2);
		main_W_com_data(0x0023, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0024, 0x0000);
		NutDelay(2);
		main_W_com_data(0x0025, 0x8000);
		NutDelay(2);
		main_W_com_data(0x004f, 0);
		main_W_com_data(0x004e, 0);
	}

	tft_set_pointer(0x22);
}

void address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

	if (x1 > get_max_x())
		x1 = get_max_x();
	if (x2 > get_max_x())
		x2 = get_max_x();
	if (y1 > get_max_y())
		y1 = get_max_y();
	if (y2 > get_max_y())
		y2 = get_max_y();

	if (controller_type == CTRL_HX8347) {
		main_W_com_data(0x02, x1 >> 8); // Column address start2
		main_W_com_data(0x03, x1); // Column address start1
		main_W_com_data(0x04, x2 >> 8); // Column address end2
		main_W_com_data(0x05, x2); // Column address end1
		main_W_com_data(0x06, y1 >> 8); // Row address start2
		main_W_com_data(0x07, y1); // Row address start1
		main_W_com_data(0x08, y2 >> 8); // Row address end2
		main_W_com_data(0x09, y2); // Row address end1
		tft_set_pointer(0x22);
	}
#ifndef __BOOTLOADER__
	else if (controller_type == CTRL_ILI9325) {

#define TS_INS_GRAM_ADX				TS_INS_GRAM_VER_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_HOR_AD
#define TS_INS_START_ADX   			TS_INS_VER_START_AD
#define TS_INS_END_ADX   			TS_INS_VER_END_AD
#define TS_INS_START_ADY   			TS_INS_HOR_START_AD
#define TS_INS_END_ADY   			TS_INS_HOR_END_AD
		main_W_com_data(TS_INS_START_ADX, x1);
		main_W_com_data(TS_INS_END_ADX, x2);
		main_W_com_data(TS_INS_GRAM_ADX, x1);

		main_W_com_data(TS_INS_START_ADY, y1);
		main_W_com_data(TS_INS_END_ADY, y2);
		main_W_com_data(TS_INS_GRAM_ADY, y1);
		tft_set_pointer(0x22);
	} else if (controller_type == CTRL_SSD1963) {

		tft_set_pointer(SSD1963_set_column_address);
		tft_write_byte(x1 >> 8);
		tft_write_byte(x1 & 0x00ff);
		tft_write_byte(x2 >> 8);
		tft_write_byte(x2 & 0x00ff);

		tft_set_pointer(SSD1963_set_page_address);
		tft_write_byte(y1 >> 8);
		tft_write_byte(y1 & 0x00ff);
		tft_write_byte(y2 >> 8);
		tft_write_byte(y2 & 0x00ff);

		tft_set_pointer(SSD1963_write_memory_start);

	}
#endif
	else if (controller_type == CTRL_SSD1289) {

		main_W_com_data(0x44, (y2 << 8) + y1);
		main_W_com_data(0x45, x1);
		main_W_com_data(0x46, x2);
		main_W_com_data(0x4e, y1);
		main_W_com_data(0x4f, x1);

		tft_set_pointer(0x22);
	}

}

void cld_write_color(uint8_t hh, uint8_t ll) {
	tft_write_word((hh << 8) | ll);
}

int get_max_x(void) {

	if (controller_type == CTRL_SSD1963)
		return TFT_1963_MAX_X;
	else
		return TFT_MAX_X;
}

int get_max_y(void) {

	if (controller_type == CTRL_SSD1963)
		return TFT_1963_MAX_Y;
	else
		return TFT_MAX_Y;
}

// RGB888 -> RGB565 converter
uint16_t tft_generate_color(uint8_t red, uint8_t green, uint8_t blue) {
 uint8_t r = ((red/255.0)*31);
 uint8_t g = ((green/255.0)*63);
 uint8_t b = ((blue/255.0)*31);

return ((r & 0x1f)<<11) | ((g & 0x3f)<<5) | (b & 0x1f); //R (5 bits) + G (6 bits) + B (5 bits)
}

void tft_pant(unsigned int color) {
	int i;
	int j;
	uint8_t d;
	uint16_t maxX, maxY;

	if (controller_type == CTRL_UNKNOWN)
		return;

	maxX = get_max_x();
	maxY = get_max_y();
	address_set(0, 0, maxX, maxY);

	// set high data byte to CPLD
	OUTB( CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, color >> 8);

	d = color & 0xff;
	for (i = 0; i < (maxX + 1); i++) {
		for (j = 0; j < (maxY + 1); j++) {
			// write low byte to LCD
			OUTB( LCD_BASE_ADDR + LCD_DATA, d);
		}
	}
}

void tft_fill_rect(uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2,
		uint16_t y2) {

	int i;
	int j;
	uint8_t d;

	if (controller_type == CTRL_UNKNOWN)
		return;

	address_set(x1, y1, x2, y2);

	// set high data byte to CPLD
	OUTB( CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, color >> 8);

	d = color & 0xff;

	for (i = y1; i <= y2; i++) {
		for (j = x1; j <= x2; j++) {
			// write low byte to LCD
			OUTB( LCD_BASE_ADDR + LCD_DATA, d);
		}
	}
}

void tft_put_flash_image(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
		uint32_t flash_address) {
#ifndef __BOOTLOADER__
	volatile uint8_t b;
	uint32_t pixel;
	uint16_t bytes16;
	uint8_t sector;
	uint16_t address;

	if (controller_type == CTRL_UNKNOWN)
		return;

	address_set(x1, y1, x2, y2);

	//enable copy mode Flash -> TFT
	OUTB(CPLD_BASE_ADDR + MODE_CTRL_ADDR, 1 << TFT_WRITE_ON_FLASH_READ);

	address = (flash_address & 0x7fff) + FLASH_BASE_ADDRESS;
	sector = (flash_address >> 15) & 0x7f;
	FLASH_SELECT_SECTOR(sector);

	pixel = (y2 - y1 + 1);
	pixel *= (x2 - x1 + 1);

	if ((pixel & 1) || (address & 1)) {
		while (pixel--) {
			// read low byte from Flash
			b = INB ( address++ );
			if (!address) {
				address = FLASH_BASE_ADDRESS;
				FLASH_SELECT_SECTOR(++sector);
			}
		}
	} else
		while (pixel > 0) {

			if ((pixel >> 1) >= 0xffff) {
				bytes16 = 0xffff;
				pixel -= 0xffff;
				pixel -= 0xffff;
			} else {
				bytes16 = pixel >> 1;
				pixel -= bytes16;
				pixel -= bytes16;
			}
			while (bytes16--) {
				// read low byte from Flash
				b = INB ( address++ );
				b = INB ( address++ );
				if (!address) {
					address = FLASH_BASE_ADDRESS;
					FLASH_SELECT_SECTOR(++sector);
				}
			}
		}

#ifdef LCD_DEBUG
	printf ("done\n");
#endif

#endif
}

void inttostr(int dd, unsigned char *str) {
	str[0] = dd / 10000 + 48;
	str[1] = (dd / 1000) - ((dd / 10000) * 10) + 48;
	str[2] = (dd / 100) - ((dd / 1000) * 10) + 48;
	str[3] = (dd / 10) - ((dd / 100) * 10) + 48;
	str[4] = dd - ((dd / 10) * 10) + 48;
	str[5] = 0;
}

//====================================================================
// Macro to access strings defined in PROGMEM above 64kB
//--------------------------------------------------------------------
#define FAR(var)                     \
({ uint_farptr_t tmp;                \
   __asm__ __volatile__(             \
       "ldi    %A0, lo8(%1)"  "\n\t" \
       "ldi    %B0, hi8(%1)"  "\n\t" \
       "ldi    %C0, hh8(%1)"  "\n\t" \
       : "=d" (tmp)                  \
       : "p"  (&(var)));             \
   tmp;                              \
})
//-------------------------------------------------------------------

static inline void showzifu(unsigned int x, unsigned int y, unsigned char value,
		unsigned int dcolor, unsigned int bgcolor) {
	uint8_t i, j;
	uint8_t by;

	if (controller_type == CTRL_UNKNOWN)
		return;

#ifdef LCD_DEBUG
	printf_P (PSTR("<%c> x=%d y=%d\n"), value, x, y);

// draw box
	tft_fill_rect ( TFT_COLOR_YELLOW, x,y,x+7,y+11);

#endif

	address_set(x, y, x + 7, y + 11);

	value -= 32;
	for (j = 0; j < 12; j++) {
		by = pgm_read_byte_far(FAR(*zifu) + value * 12 + j);
		for (i = 0; i < 8; i++) {
			if ((by & (1 << (7 - i))) != 0) {
				tft_write_word(dcolor);
			} else {
				tft_write_word(bgcolor);
			}
		}
	}
}

unsigned int showzifustr(unsigned int x, unsigned int y, unsigned char *str,
		unsigned int dcolor, unsigned int bgcolor) {
	unsigned int x1, y1;
	x1 = x;
	y1 = y;
	while (*str != '\0') {
		showzifu(x1, y1, *str, dcolor, bgcolor);
		x1 += 7;
		str++;
	}
	return x1;
}

//**********************************************************
void spistar(void) //SPI��ʼ
{
	TOUCH_SET_CS
	TOUCH_SET_CLK
	TOUCH_SET_DIN
}
//**********************************************************
void WriteCharTo7843(uint8_t num) //SPIд���
{
	uint8_t count = 0;

	for (count = 0; count < 8; count++) {
		//num<<=1;
		//DIN=CY;
		if (num & 0x80)
			TOUCH_SET_DIN
		else
			TOUCH_CLR_DIN
		num <<= 1;
		//DCLK=0; _nop_();_nop_();_nop_();                //��������Ч
		TOUCH_CLR_CLK
		nop();
		nop();
		nop();
		//DCLK=1; _nop_();_nop_();_nop_();
		TOUCH_SET_CLK
		nop();
		nop();
		nop();
	}
}
//**********************************************************
uint16_t ReadFromCharFrom7843(void) {
	uint8_t count = 0;
	uint16_t Num = 0;

	for (count = 0; count < 12; count++) {
		Num <<= 1;
		TOUCH_SET_CLK
		nop();
		nop();
		nop();
		TOUCH_CLR_CLK
		nop();
		nop();
		nop();

		if (TOUCH_DOUT)
			Num++;
	}
	return Num;
}

void AD7843(void) {
	TOUCH_CLR_CS
	WriteCharTo7843(0x90);
	TOUCH_SET_CLK
	TOUCH_CLR_CLK
	TP_X = ReadFromCharFrom7843();

	WriteCharTo7843(0xD0);
	TOUCH_SET_CLK
	TOUCH_CLR_CLK
	TP_Y = ReadFromCharFrom7843();
	//CS=1;
	TOUCH_SET_CS
}

/** \brief Sets TFT backlight to full intensity
 *
 * Sets the TFT backlight to full intensity and clear the sleep_timer variable
 */
void set_backlight_on() {
	// set display to full intensity
	TFT_BACKLIGHT_ON
	sleep_timer = 0;

}

#ifndef __BOOTLOADER__
THREAD(poll_touch , arg) {

//unsigned char ss[6];

	NutThreadSetPriority(100);

	// prevent touch events during init phase
	NutSleep(300);
	while (TOUCH_IRQ == 0)
		AD7843();

	current_touch_state = 0;
	touch_event.state = IDLE;
	sleep_timer = 0;
	long_down_flag = 0;
	touch_timer = 0;

	/*
	 * Now loop endless for next touch event
	 */

	for (;;) {

		NutSleep(20);

		if (TOUCH_IRQ == 0) {
			// the touch screen is touched

			// get new position
			AD7843();

			// set display to full intensity
			TFT_BACKLIGHT_ON
			sleep_timer = 0;

			if (controller_type != CTRL_SSD1963) {
				ly = (TP_Y - SSD1289_Y_OFFSET) / SSD1289_Y_OFFSET_FACT;
				if (ly < 0)
					ly = 0;
				if (ly > get_max_y())
					ly = get_max_y();

				if (invert_touch_y)
					ly = get_max_y() - ly;

				//The 2.4" TFT using ILI9325 has mirrored X-Coordinates!
				if ((controller_type == CTRL_ILI9325) ^ invert_touch_x) {

					if (TP_X >= 380) {
						lx = (3910 - TP_X) / 11;
						if (lx < 0)
							lx = 0;
					} else
						lx = (TP_X - 391) / 11;

				} else
					lx = (TP_X - SSD1289_X_OFFSET) / SSD1289_X_OFFSET_FACT;
			} else {
#ifdef TFT_480_800_CTRL_SSD1963
				// here we treat the 5" display using SSD1963

				ly= (TP_X - SSD1963_50_Y_OFFSET) / SSD1963_50_Y_OFFSET_FACT;
				if (!invert_touch_y)
					ly = get_max_y() - ly;
				if (ly < 0)
					ly = 0;
				if (ly > get_max_y())
					ly = get_max_y();

				lx= (SSD1963_50_X_OFFSET - TP_Y) / SSD1963_50_X_OFFSET_FACT;
	#ifdef TFT_480_800_CTRL_SSD1963_INVERT_TOUCH_X
				if (!invert_touch_x) {	// for the new 5.0" modules sold since b/o 2013
	#else
				if (invert_touch_x) { // for the 5.0" modules sold until b/o 2013
	#endif //TFT_480_800_CTRL_SSD1963_INVERT_TOUCH_X
					lx= get_max_x() - lx;
				}
				if (lx < 0)
					lx = 0;
#else
				// here we treat the 4.3" display using SSD1963

	#ifdef TFT_272_480_CTRL_SSD1963_NEW
				ly = (TP_X - SSD1963_43_Y_OFFSET) / SSD1963_43_Y_OFFSET_FACT;	// for the new 4.3" modules with SD slot
	#else
				ly = (TP_Y - SSD1963_43_Y_OFFSET) / SSD1963_43_Y_OFFSET_FACT;	// for the 4.3" modules without SD slot
	#endif
				if (invert_touch_y)
					ly = get_max_y() - ly;
				if (ly < 0)
					ly = 0;
				if (ly > get_max_y())
					ly = get_max_y();
	#ifdef TFT_272_480_CTRL_SSD1963_NEW
				lx = (TP_Y - SSD1963_43_X_OFFSET) / SSD1963_43_X_OFFSET_FACT;	// for the new 4.3" modules with SD slot
	#else
				lx = (TP_X - SSD1963_43_X_OFFSET) / SSD1963_43_X_OFFSET_FACT;	// for the 4.3" modules without SD slot
	#endif
				if (invert_touch_x) {
					lx = get_max_x() - lx;
				}
				if (lx < 0)
					lx = 0;
				if (lx > get_max_x())
					lx = get_max_x();

#endif // TFT_480_800_CTRL_SSD1963
			}

			if (lx > get_max_x())
				lx = get_max_x();

#ifdef TOUCH_DEBUG
			printf_P (PSTR("X=%d - Y=%d - lx=%d - ly=%d\n"), TP_X, TP_Y, lx, ly);
#endif
			touch_event.lx = lx;
			touch_event.ly = ly;
			if (current_touch_state) {
				touch_event.state = MOVE;
				++touch_timer;
				if ((!long_down_flag) && (touch_timer > MAX_SHORT_TIME)) {
					long_down_flag = 1;
					touch_event.state = TOUCHED_LONG;
				}
				if ((touch_timer == AUTO_TIME_FAST1)
						|| (touch_timer == AUTO_TIME_FAST2)
						|| (touch_timer == AUTO_TIME_FAST3)) {
					touch_event.state = TOUCHED_AUTO_FAST;
				}
				if (touch_timer == AUTO_TIME) {
					touch_timer = 0;
					touch_event.state = TOUCHED_AUTO;
				}
			} else
				touch_event.state = TOUCHED;
			// send position to main loop
			process_touch_event(&touch_event);

			current_touch_state = TOUCH_RELEASE_TIME;

		} else {
			if (current_touch_state) {
				if (!(--current_touch_state)) {
					// now the touch screen is no more touched
					if (long_down_flag) {
						touch_event.state = RELEASED_LONG;
						process_touch_event(&touch_event);
					} else {
						touch_event.state = TOUCHED_SHORT;
						process_touch_event(&touch_event);
					}
					touch_event.state = RELEASED;
					process_touch_event(&touch_event);
					long_down_flag = 0;
					touch_timer = 0;
				}
			}
			// dimm display if not touched for TIME_TO_SLEEP
			if ((sleep_timer < COUNT_TO_SLEEP)
					&& (system_page_active != SYSTEM_PAGE_BUSMON)
					&& (system_page_active != SYSTEM_PAGE_HARDWARE_MONITOR))
				if (++sleep_timer == COUNT_TO_SLEEP)
					TFT_BACKLIGHT_DIMM
		}

	}

}

#else

uint8_t check_touch_event () {

	if (TOUCH_IRQ==0) {
		// the touch screen is touched

		// get new position
		AD7843();

		ly= (TP_Y-200) / 15;
		if (ly < 0) ly = 0;
		if (ly > get_max_y()) ly = get_max_y();

		lx= (TP_X-350) / 11;
		if (lx > get_max_x()) lx = get_max_x();

		return 1;
	}

	return 0;
}

uint8_t touched_inside_bounds (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {

	if (lx < x1) return 0;
	if (lx > x2) return 0;
	if (ly < y1) return 0;
	if (ly > y2) return 0;

	return 1;
}

#endif

// refresh backlight setting as dimming values are changed
void refresh_backlight(void) {

	if ((sleep_timer == COUNT_TO_SLEEP)
			&& (system_page_active != SYSTEM_PAGE_BUSMON)) {
		TFT_BACKLIGHT_DIMM
	}
	if ((sleep_timer < COUNT_TO_SLEEP)
			&& (system_page_active != SYSTEM_PAGE_BUSMON)) {
		TFT_BACKLIGHT_ON
	}
}

//init the tft control i/f
void touch_init(void) {

	char_x = START_CHAR_X_POS;
	char_y = START_CHAR_Y_POS;

#ifndef __BOOTLOADER__
	// create touch poll process
	NutThreadCreate("TOUCH", poll_touch, 0, NUT_THREAD_POLL_TOUCH_STACK);
#endif
}

//init the tft control i/f
void tft_init(void) {

	// init tft control ports
	TFT_BACKLIGHT_OFF
	TFT_BACKLIGHT_INIT

	// init touch controller ports
	TOUCH_PORT_INIT
	spistar(); //start spi

#ifdef __BOOTLOADER__
	// enable external memory-if
	// enable memory i/f
	MCUCR |= (1 << SRE);
	XMCRA &= 0xff ^ (1<<SRW11);// no wait
	MCUCR &= 0xff ^ (1<<SRW10);// no wait
#endif

	tft_init_sequence();

	tft_pant(0xffff);

	TFT_BACKLIGHT_ON

}

// set cursor
void tft_set_cursor(uint16_t x, uint8_t y) {

	char_x = x;
	char_y = y;
}

// clear screen and reset cursor
void tft_clrscr(uint16_t ccolor) {

	char_x = START_CHAR_X_POS;
	char_y = START_CHAR_Y_POS;
	tft_pant(ccolor);
}

#ifndef __BOOTLOADER__

#define BUFF_SIZE	60
unsigned char buffer[BUFF_SIZE];


/**
 * print out system text
 *
 * This function prints out text on the TFT display using the system font.
 * The output position is determined by char_x and char_y. Formatting
 * options of sprintf are provided.
 *
 * @param ccolor uint16_t defining the character color
 * @param bcolor uint16_t defining the background color
 * @param *format const char formatting string, see sprintf
 * @return int number of characters written to the screen
 *
 */
int printf_tft(uint16_t ccolor, uint16_t bcolor, const char *format, ...) {

	int c;
	va_list args;

	va_start(args, format);
	c = vsprintf((char*) buffer, format, args);
	va_end(args);

#ifdef LCD_DEBUG
	printf_P (PSTR("%s\n"), buffer);
#endif

	showzifustr(char_x, char_y, buffer, ccolor, bcolor);

	char_x = START_CHAR_X_POS;
	char_y += CHAR_LINE_SPACING;
	if (char_y > END_CHAR_Y_POS)
		char_y = START_CHAR_Y_POS;

	return c;
}


/**
 * print out system text from Program Memory
 *
 * This function prints out text on the TFT display using the system font.
 * The output position is determined by char_x and char_y. Formatting
 * options of sprintf are provided.
 *
 * @param ccolor uint16_t defining the character color
 * @param bcolor uint16_t defining the background color
 * @param *format const char formatting string, see sprintf
 * @return int number of characters written to the screen
 *
 */
int printf_tft_P(uint16_t ccolor, uint16_t bcolor, const char *format, ...) {

	int c;
	va_list args;

	va_start(args, format);
	c = vsprintf_P((char*) buffer, format, args);
	va_end(args);

#ifdef LCD_DEBUG
	printf_P (PSTR("%s\n"), buffer);
#endif

	showzifustr(char_x, char_y, buffer, ccolor, bcolor);

	char_x = START_CHAR_X_POS;
	char_y += CHAR_LINE_SPACING;
	if (char_y > END_CHAR_Y_POS)
		char_y = START_CHAR_Y_POS;

	return c;
}

/**
 * print out system text from Program Memory to absolute position
 *
 * This function prints out text on the TFT display using the system font.
 * The output position is determined by char_x and char_y. Formatting
 * options of sprintf are provided.
 *
 * @param abs_x unsigned int defines the absolute x position to print
 * @param abs_y unsigned int defines the absolute y position to print
 * @param ccolor uint16_t defining the character color
 * @param bcolor uint16_t defining the background color
 * @param *format const char formatting string, see sprintf
 * @return int number of characters written to the screen
 *
 */
int printf_tft_absolute_P(unsigned int abs_x, unsigned int abs_y, uint16_t ccolor, uint16_t bcolor, const char *format, ...) {

	int c;
	va_list args;

	va_start(args, format);
	c = vsprintf_P((char*) buffer, format, args);
	va_end(args);

#ifdef LCD_DEBUG
	printf_P (PSTR("%s\n"), buffer);
#endif

	showzifustr(abs_x, abs_y, buffer, ccolor, bcolor);

	return c;
}

/**
 * print out system text from Program Memory, append to existing position
 * no line warp until now!!
 *
 * This function prints out text on the TFT display using the system font.
 * The output position is determined by char_x and char_y. Formatting
 * options of sprintf are provided.
 *
 * @param ccolor uint16_t defining the character color
 * @param bcolor uint16_t defining the background color
 * @param *format const char formatting string, see sprintf
 * @return int number of characters written to the screen
 *
 */
int printf_tft_append(uint16_t ccolor, uint16_t bcolor, const char *format, ...) {

	int c;
	va_list args;

	va_start(args, format);
	c = vsprintf((char*) buffer, format, args);
	va_end(args);

#ifdef LCD_DEBUG
	printf_P (PSTR("%s\n"), buffer);
#endif

	char_x = showzifustr(char_x, char_y, buffer, ccolor, bcolor);

	if (char_x > END_CHAR_X_POS)
		char_x = START_CHAR_X_POS;
	//char_y += CHAR_LINE_SPACING;
	if (char_y > END_CHAR_Y_POS)
		char_y = START_CHAR_Y_POS;

	return c;
}
#endif
