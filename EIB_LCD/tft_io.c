/**
 * \file tft_io.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_io.h"
#include "tft_hx8347a_32_0.h"
#include "tft_ili9325_24_0.h"
#include "tft_ssd1289_32_0.h"
#include "tft_ssd1963_50_0.h"
#include "tft_ssd1963_50_1.h"
#include "tft_ssd1963_70_0.h"
#include "tft_ssd1963_43_0.h"
#include "tft_ssd1963_43_1.h"

#include "NandFlash.h"
#include <stdio.h>
#include <stdarg.h>
#include <avr/pgmspace.h>
#include "charset.h"

t_touch_event touch_event;
volatile uint8_t backlight_dimming;
volatile uint8_t backlight_active;
volatile uint8_t controller_type; // 0=unknown, 1 = HX8347-A, 2=SSD1289, 3=ILI9325 2.4" 180� rotated, 4=ssd1963
volatile uint16_t controller_id;
volatile uint8_t lcd_type;		 // Resistor coding used to distinguish between LCDs if controller type is same
volatile uint8_t invert_touch_y; // invert X coordinate of touch position
volatile uint8_t invert_touch_x; // invert Y coordinate of touch position
volatile uint16_t screen_max_x; // max X coordinate of display
volatile uint16_t screen_max_y; // max Y coordinate of display
volatile uint8_t rotate; // rotate screen by 180�
void (*drv_convert_touch_coordinates)(void);	// this function handles the touch event for the TFT in use
void (*drv_address_set)(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void (*drv_lcd_rotate)(uint8_t rotation);
volatile int16_t lx, ly;
volatile int16_t TP_X, TP_Y;

uint8_t current_touch_state; // 0=not touched
uint16_t sleep_timer; // timer to set display into sleep mode
uint16_t touch_timer; // timer to measure the down time
uint8_t long_down_flag; // touched for long time

uint16_t char_x, char_y; // Position of next character for system text output

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


/* sends the init sequence to the tft controller
 */
void tft_init_sequence(void) {

	// Read resistor coding, if any
	lcd_type = check_lcd_type_code ();

	controller_type = CTRL_UNKNOWN;

	while (controller_type == CTRL_UNKNOWN) {
		tft_set_reset_inactive();
		NutDelay(6);
		tft_set_reset_active();
		NutDelay(10);
		tft_set_reset_inactive();
		NutDelay(20);

		// autodetect sequence:
		controller_id = main_W_com_read_data(0);
		// get R00
#ifdef LCD_DEBUG
		printf_P(PSTR("\nR00= %4.4x  LCD-R-Code= %u"), controller_id, lcd_type);
#endif
		if ((controller_id & HX8347_MASK) == HX8347_R00) {

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
/*
#ifdef LCD_DEBUG
			printf_P(PSTR("\nSSD1963: %2.2x %2.2x %2.2x %2.2x %2.2x\n"), b0, b1, b2, b3, b4);
			printf_P(PSTR("SSD1963: %2.2x %2.2x %2.2x %2.2x %2.2x\n"), SSD1963_SSL_H, SSD1963_SSL_L, SSD1963_PROD, SSD1963_REV, SSD1963_EXIT);
#endif
*/
			if ((b0 == SSD1963_SSL_H) && (b1 == SSD1963_SSL_L)
					&& (b2 == SSD1963_PROD) && (b3 == SSD1963_REV)
					&& (b4 == SSD1963_EXIT)) {
				controller_type = CTRL_SSD1963;
			} else
				controller_type = CTRL_HX8347;
		}
		else if ((controller_id & ILI9325_MASK) == ILI9325_R00) {
			controller_type = CTRL_ILI9325;
		}
		else if ((controller_id & SSD1289_MASK) == SSD1289_R00) {
			controller_type = CTRL_SSD1289;
		}

	}
#ifdef LCD_DEBUG
	printf_P(PSTR("\nController-ID= %d"), controller_type);
#endif

	if (controller_type == CTRL_HX8347) {
		hx8347a_32_0_init();
	}

	else if (controller_type == CTRL_ILI9325) {
		ili9325_24_0_init();
	}

	else if (controller_type == CTRL_SSD1963) {

		switch (lcd_type) {

			case SSD1963_TYPE_0: ssd1963_50_0_init();
				break;

			case SSD1963_TYPE_1: ssd1963_50_1_init();
				break;

			case SSD1963_TYPE_2: ssd1963_43_0_init();
				break;

			case SSD1963_TYPE_3: ssd1963_70_0_init();
				break;

			case SSD1963_TYPE_4: ssd1963_43_1_init();
				break;
		}
	}

	else if (controller_type == CTRL_SSD1289) {
		ssd1289_32_0_init();
	}

	tft_set_pointer(0x22);

#ifdef LCD_DEBUG
	printf_P(PSTR("Resolution %u x %u\n====================\n"), get_max_x()+1, get_max_y()+1);
#endif
}


void address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

	if (drv_address_set == NULL)
		return;

	if (x1 > get_max_x())
		x1 = get_max_x();
	if (x2 > get_max_x())
		x2 = get_max_x();
	if (y1 > get_max_y())
		y1 = get_max_y();
	if (y2 > get_max_y())
		y2 = get_max_y();

	drv_address_set (x1, y1, x2, y2);

}

void cld_write_color(uint8_t hh, uint8_t ll) {
	tft_write_word((hh << 8) | ll);
}

inline int16_t get_max_x(void) {
	return screen_max_x;
}

inline int16_t get_max_y(void) {
	return screen_max_y;
}

// RGB888 -> RGB565 converter
uint16_t tft_generate_color(uint8_t red, uint8_t green, uint8_t blue) {
 uint8_t r = ((red/255.0)*31);
 uint8_t g = ((green/255.0)*63);
 uint8_t b = ((blue/255.0)*31);

return ((r & 0x1f)<<11) | ((g & 0x3f)<<5) | (b & 0x1f); //R (5 bits) + G (6 bits) + B (5 bits)
}

/**
 * Clears the total screen by filling it with the specified color
 */
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

THREAD(poll_touch , arg) {

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

			// convert touch coordinates TP_X and TP_Y into screen coordinates lx, ly
			drv_convert_touch_coordinates ();

			// Make sure touch event is within screen limits
			if (ly < 0)
				ly = 0;
			else if (ly > get_max_y())
				ly = get_max_y();
			if (invert_touch_y)
				ly = get_max_y() - ly;

			// The check for lx < 0 should be implemented in the panel specific driver part,
			// because the symbol area of the small size panels is represented by negative lx values.
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

	// create touch poll process
	NutThreadCreate("TOUCH", poll_touch, 0, NUT_THREAD_POLL_TOUCH_STACK);
}

//init the tft control i/f
void tft_init(void) {

	// init tft control ports
	TFT_BACKLIGHT_OFF
	TFT_BACKLIGHT_INIT

	// make sure we don't point into the forest ;-)
    drv_address_set = NULL;
	drv_convert_touch_coordinates = NULL;
    drv_lcd_rotate = NULL;

	// init touch controller ports
	TOUCH_PORT_INIT
	spistar(); //start spi

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
