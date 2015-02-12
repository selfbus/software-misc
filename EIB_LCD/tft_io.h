/**
 * \file tft_io.h
 *
 * \brief This module contains constant definitions for the TFT LCD access
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
#ifndef TFT_IO_H_
#define TFT_IO_H_

#include <io.h>
#include <sys/timer.h>
#include <sys/thread.h>
#include "MemoryMap.h"
#include "ssd1963_cmd.h"


// TFT modules using the same controller
#define SSD1963_TYPE_0	0		// 5.0" 800 x 480 display with SD Slot
#define SSD1963_TYPE_1	1		// 5.0" 800 x 480 display (old version)
#define SSD1963_TYPE_2	2		// 4.3" 480 x 272 display with SD Slot
#define SSD1963_TYPE_3	3		// 7.0" 800 x 480 display with SD Slot
#define SSD1963_TYPE_4	4		// 4.3" 480 x 272 display without SD Slot


#define	HX8347_MASK  0xffff
#define	HX8347_R00	 0x0000

#define	ILI9325_MASK 0xffff
#define	ILI9325_R00	 0x9325

#define	SSD1289_MASK 0xff00
#define	SSD1289_R00	 0x8900

#define	SSD1963_MASK 0xffff
#define	SSD1963_R00	 0x1963

//values for controller_type
#define CTRL_UNKNOWN	0	//unknown
#define CTRL_HX8347		1	//HX8347-A
#define CTRL_SSD1289	2	//SSD1289
#define CTRL_ILI9325	3	//ILI9325 2.4" 180� rotated
#define CTRL_SSD1963	4	//SSD1963 5.0"

#define LCD_POINTER	0x00
#define LCD_DATA	0x01

#define	TFT_RESET_PORT	PORTF
#define	TFT_RESET_DDR	DDRF
#define	TFT_RESET_BIT	1
#define	TFT_SET_RESET_ACTIVE	TFT_RESET_DDR |= (1<<TFT_RESET_BIT); TFT_RESET_PORT &= 0xff ^ (1<<TFT_RESET_BIT);
#define	TFT_SET_RESET_INACTIVE	TFT_RESET_DDR |= (1<<TFT_RESET_BIT); TFT_RESET_PORT |= (1<<TFT_RESET_BIT);

#define	TFT_BACKLIGT_CTRL_PORT	PORTB
#define	TFT_BACKLIGT_CTRL_DDR	DDRB
#define TFT_BACKLIGT_CTRL_BIT	7

// intensity of dimmed backlight
extern volatile uint8_t backlight_dimming;
extern volatile uint8_t backlight_active;
#define TFT_BACKLIGHT_INIT	TFT_BACKLIGT_CTRL_DDR = (1<<TFT_BACKLIGT_CTRL_BIT);	TCCR2 = (1 << WGM20) | (1 << WGM21) | (1 << COM21) | (1 << CS22);
#define TFT_BACKLIGHT_FAST_PWM	TCCR2 = (1 << WGM20) | (1 << WGM21) | (1 << COM21) | (1 << CS21);
#define TFT_BACKLIGHT_SLOW_PWM	TCCR2 = (1 << WGM20) | (1 << WGM21) | (1 << COM21) | (1 << CS22);
#define TFT_BACKLIGHT_ON	OCR2 = backlight_active;
#define TFT_BACKLIGHT_OFF	OCR2 = 0x00;
#define TFT_BACKLIGHT_DIMM	OCR2 = backlight_dimming;

#define D_CLK_PORT		PORTD
#define D_CLK_DDR		DDRD
#define D_CLK_BIT		0
#define D_CS_PORT		PORTD
#define D_CS_DDR		DDRD
#define D_CS_BIT		1
#define D_DIN_PORT		PORTE
#define D_DIN_DDR		DDRE
#define D_DIN_BIT		5
#define D_DOUT_PORT		PINE
#define D_DOUT_BIT		4
#define D_IRQ_PORT		PINE
#define D_IRQ_BIT		6
#define TOUCH_PORT_INIT		D_CLK_DDR |= (1<<D_CLK_BIT); D_CS_DDR |= (1<<D_CS_BIT); D_DIN_DDR |= (1<<D_DIN_BIT);
#define	TOUCH_SET_CLK	D_CLK_PORT |= (1<<D_CLK_BIT);
#define	TOUCH_CLR_CLK	D_CLK_PORT &= 0xff ^(1<<D_CLK_BIT);
#define	TOUCH_SET_CS	D_CS_PORT |= (1<<D_CS_BIT);
#define	TOUCH_CLR_CS	D_CS_PORT &= 0xff ^(1<<D_CS_BIT);
#define	TOUCH_SET_DIN	D_DIN_PORT |= (1<<D_DIN_BIT);
#define	TOUCH_CLR_DIN	D_DIN_PORT &= 0xff ^(1<<D_DIN_BIT);
#define TOUCH_DOUT		(D_DOUT_PORT & (1<<D_DOUT_BIT))
#define TOUCH_IRQ		(D_IRQ_PORT & (1<<D_IRQ_BIT))

#define nop() \
   asm volatile ("nop")

// Some RGB 565 color codes
#define TFT_COLOR_WHITE		0xffff
#define TFT_COLOR_LIGHTGRAY	0xbdf7
#define TFT_COLOR_RED		0xf800
#define TFT_COLOR_ORANGE	0xfd20
#define TFT_COLOR_YELLOW	0xffe0
#define TFT_COLOR_GREEN		0x07e0
#define TFT_COLOR_BLUE		0x001F
#define TFT_COLOR_DS_BLUE	0x05FF
#define TFT_COLOR_PURPLE	0xA11E
#define TFT_COLOR_BLACK		0x0000

// text IO constants
#define START_CHAR_X_POS	1
#define START_CHAR_Y_POS	1
#define END_CHAR_X_POS		get_max_x()
#define END_CHAR_Y_POS		200
#define CHAR_LINE_SPACING	12

#define TOUCH_RELEASE_TIME	3	// units of touch polling loop ticker
// IDLE: 			nothing is touched
// TOUCHED: 		touch panel is just touched
// TOUCHED_SHORT:	panel was touched for less than MAX_SHORT_TIME and is now released
// TOUCHED_LONG:	panel is touched for more than MAX_SHORT_TIME and still hold
// TOUCHED_AUTO:	panel is still touched and now autorepeat is triggered after AUTO_TIME
// MOVE:			touch panel is still touched, new position is sent
// RELEASED: 		touch panel is just released
// RELEASED_LONG:	touch panel was pressed for "long" time ans is released now
// TOUCHED_AUTO_FAST:	fast autorepeat for edit buttons

//						0	1			2				3				4		5		6			7				8
enum _TOUCH_STATES { IDLE, TOUCHED, TOUCHED_SHORT, TOUCHED_LONG, TOUCHED_AUTO, MOVE, RELEASED, RELEASED_LONG, TOUCHED_AUTO_FAST };
typedef enum _TOUCH_STATES t_touch_states;
// base time is 20ms
#define MAX_SHORT_TIME		15	// 300ms for short/long
#define AUTO_TIME			100	// 2sec for autorepeat interval
#define AUTO_TIME_FAST1		33	// 660msec for fast autorepeat interval
#define AUTO_TIME_FAST2		66	// 1.2sec for fast autorepeat interval
#define AUTO_TIME_FAST3		99	// 1.9sec for fast autorepeat interval
#define TOUCH_MARGIN		60	// aura for touched elements to keep the function trigger
// interval [ms] for polling the touch controller
#define TOUCH_POLL_INTERVAL	20
// timeout [s] to set LCD and touch to idle mode
#define TIME_TO_SLEEP		30
#define COUNT_TO_SLEEP		(TIME_TO_SLEEP * 1000 / TOUCH_POLL_INTERVAL)

typedef struct __attribute__ ((packed)) {
t_touch_states	state;
int lx;
int	ly;
} t_touch_event;

/*
* These variables permit communication between the common TFT driver part in tft_io and the panel
* specific driver modules.
*  ==============================================================================
*/

extern volatile uint8_t controller_type;// 0=unknown, 1 = HX8347-A, 2=SSD1289, 3=ILI9325 2.4" 180° rotated
extern volatile uint16_t controller_id; // R00 value
extern volatile uint8_t lcd_type;		// Resistor coding used to distinguish between LCDs if controller type is same
extern volatile uint8_t invert_touch_y; // invert X coordinate of touch position
extern volatile uint8_t invert_touch_x; // invert Y coordinate of touch position
extern volatile uint8_t lcd_rotation;   // hardware 180° rotation of LCD screen (needs support of LCD controller!)

// global variables used by the TFT base driver
extern volatile uint16_t screen_max_x;  // max X coordinate of display
extern volatile uint16_t screen_max_y;  // max Y coordinate of display
extern volatile int16_t lx, ly;         // screen coordinates of touch event
extern volatile int16_t TP_X, TP_Y;	    // absolute coordinates of touch event

// this function handles the touch event for the TFT in use
extern void (*drv_convert_touch_coordinates)(void);
// this function sets the address pointer for the next display data write operation
extern void (*drv_address_set)(unsigned int, unsigned int, unsigned int, unsigned int);
extern void (*drv_lcd_rotate)(uint8_t rotation);

/*
*  ===================== End of driver communication block ======================
*/

void tft_set_reset_active (void);
void tft_set_reset_inactive (void);

/** writes a byte to the tft-lcd
* d: data byte
*/
void tft_write_byte (uint8_t);

/** writes a word to the tft-lcd
* d: data byte
*/
void tft_write_word (uint16_t);

/** This function writes to the tft pointer register
*/
void tft_set_pointer (uint8_t);

/** controls the ft backlight
* light = 0: off
* light = 1: on
*/
void tft_backlight (uint8_t);

// refresh backlight setting as dimming values are changed
void refresh_backlight(void);

/** sents the init sequence to the tft controller
* already called by tft_init();
*/
void tft_init_sequence (void);

//init the tft control i/f
void tft_init (void);
//init the touch control i/f
void touch_init (void);

// print string to screen
//unsigned int showzifustr(unsigned int x,unsigned int y,unsigned char *str,unsigned int dcolor,unsigned int bgcolor)
unsigned int showzifustr(unsigned int,unsigned int,unsigned char*,unsigned int,unsigned int);
//void showzifustr2(unsigned int,unsigned int,unsigned char*,unsigned int,unsigned int);
void inttostr(int,unsigned char*);

int printf_tft(uint16_t, uint16_t, const char* , ...);			// Regular
int printf_tft_P(uint16_t, uint16_t, const char* , ...);		// From FLASH
int printf_tft_absolute_P(unsigned int, unsigned int, uint16_t, uint16_t, const char *, ...);	// Abolute position
int printf_tft_append(uint16_t, uint16_t, const char* , ...);	// Append to current position

void tft_clrscr (uint16_t);
void tft_set_cursor (uint16_t, uint8_t);

/** RGB888 -> RGB565 converter
 *  RED = 8bit red
 *  GREEN = 8bit green
 *  BLUE = 8bit blue
 */
uint16_t tft_generate_color(uint8_t, uint8_t, uint8_t);

/** fill total screen with RGB color
 *
 */
void tft_pant(unsigned int);
/** copy image from Flash to screen
 *
 */
void tft_put_flash_image (uint16_t,uint16_t,uint16_t,uint16_t, uint32_t);
/** fill rect with color
 *
 */
void tft_fill_rect (uint16_t, uint16_t,uint16_t,uint16_t,uint16_t);

/** sent data to tft
 *
 */
void main_W_com_data(uint8_t, uint16_t);


// Used to outsource driver

/* This function reads a byte from the tft data register
 */
uint8_t tft_read_byte(void);

// get max display dimensions for the TFT screen
inline int16_t get_max_x(void);
inline int16_t get_max_y(void);
// set backlight to full intensity
void set_backlight_on (void);

#endif //TFT_IO_H_
