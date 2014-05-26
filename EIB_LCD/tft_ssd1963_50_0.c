/**
 * \file tft_ssd1963_50_0.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 * Controller:	SSD1963
 * Size:		5.0" (new version, glass LM50TQ142-A)
 * Resolution:	800 x 480
 * LCD Module:	VT800480S01-G500
 * Color depth: 16.7M
 * DCLK clock:	33.3MhHz typical
 * Order codes:	221044250453
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_ssd1963_50_0.h"


void ssd1963_50_0_address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

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


void ssd1963_50_0_convert_touch_coordinates (void) {

	ly= (TP_X - SSD1963_0_50_Y_OFFSET) / SSD1963_0_50_Y_OFFSET_FACT;

	lx= (TP_Y - SSD1963_0_50_X_OFFSET) / SSD1963_0_50_X_OFFSET_FACT;
	if (!invert_touch_x)	// needs to be inversed somehow?? Shouldn't it be like type 2
		lx = get_max_x() - lx;

	if (lx < 0)
		lx = 0;
}


void ssd1963_50_0_rotate(uint8_t rotation)
{
    tft_set_pointer(SSD1963_set_address_mode);//rotation
    if (rotation) tft_write_byte(0x0000);   // Upside down
    else tft_write_byte(0x0003);            // Back to normal
}


void ssd1963_50_0_init() {

	// set global information
	drv_convert_touch_coordinates = ssd1963_50_0_convert_touch_coordinates;
	drv_address_set = ssd1963_50_0_address_set;
    drv_lcd_rotate = ssd1963_50_0_rotate;
	// Return used resolution
	screen_max_x = T0_HDP;	// X
	screen_max_y = T0_VDP;	// Y
	//rotate = !rotate; // this display is 180� rotated

	// enable wait
	XMCRA |= (1<<SRW00) | (1<<SRW01);	// wait

	tft_set_pointer(SSD1963_set_pll_mn);	//PLL multiplier, set PLL clock to 120M
	tft_write_byte(0x0023);					// N=0x36 for 6.5M, 0x23 for 10M crystal
	tft_write_byte(0x0002);					// M = 3
	tft_write_byte(0x0004);					// Use M and N
	tft_set_pointer(SSD1963_set_pll);		// PLL enable
	tft_write_byte(0x0001);
	NutDelay(1);
	tft_set_pointer(SSD1963_set_pll);
	tft_write_byte(0x0003);
	NutDelay(5);
	tft_set_pointer(SSD1963_soft_reset);	// software reset
	NutDelay(5);

	// disable wait
	XMCRA &= ~((1<<SRW00) | (1<<SRW01));	// wait

	tft_set_pointer(SSD1963_set_lshift_freq);//PLL setting for PCLK, depends on resolution
	tft_write_byte(0x0004);				// Typical 33MHz
	tft_write_byte(0x0066);				// For 120MHz PLL
	tft_write_byte(0x0065);				// 2^20*33/120 -1

	tft_set_pointer(SSD1963_set_lcd_mode);	//LCD SPECIFICATION
	tft_write_byte(0x0027);
	tft_write_byte(0x0000);
	tft_write_byte((T0_HDP>>8)&0X00FF);		//Set HDP
	tft_write_byte(T0_HDP&0X00FF);
	tft_write_byte((T0_VDP>>8)&0X00FF);		//Set VDP
	tft_write_byte(T0_VDP&0X00FF);
	tft_write_byte(0x0000);
    NutDelay(5);

	tft_set_pointer(SSD1963_set_hori_period);//HSYNC
	tft_write_byte((T0_HT>>8)&0X00FF);		//Set HT
	tft_write_byte(T0_HT&0X00FF);
	tft_write_byte((T0_HPS>>8)&0X00FF);		//Set HPS
	tft_write_byte(T0_HPS&0X00FF);
	tft_write_byte(T0_HPW);					//Set HPW
	tft_write_byte((T0_LPS>>8)&0X00FF);		//Set HPS
	tft_write_byte(T0_LPS&0X00FF);
	tft_write_byte(0x0000);

	tft_set_pointer(SSD1963_set_vert_period);//VSYNC
	tft_write_byte((T0_VT>>8)&0X00FF);		//Set VT
	tft_write_byte(T0_VT&0X00FF);
	tft_write_byte((T0_VPS>>8)&0X00FF);		//Set VPS
	tft_write_byte(T0_VPS&0X00FF);
	tft_write_byte(T0_VPW);					//Set VPW
	tft_write_byte((T0_FPS>>8)&0X00FF);		//Set FPS
	tft_write_byte(T0_FPS&0X00FF);

	tft_set_pointer(SSD1963_set_gpio_value);
	tft_write_byte(0x0000);					//GPIO[3:0] Low

	tft_set_pointer(SSD1963_set_gpio_conf);
	tft_write_byte(0x0000);					//GPIO[3:0]=input
	tft_write_byte(0x0001);					//GPIO0 normal

	tft_set_pointer(SSD1963_set_address_mode);//rotation
	//if (rotate)
    tft_write_byte(0x0003);
	//else tft_write_byte(0x0000);

	tft_set_pointer(SSD1963_set_pixel_data_interface);//pixel data interface
	tft_write_byte(0x0003);

	NutDelay(5);

	tft_set_pointer(SSD1963_set_display_on); //display on

	tft_set_pointer(SSD1963_set_dbc_conf);
	tft_write_byte(0x000d);
}
