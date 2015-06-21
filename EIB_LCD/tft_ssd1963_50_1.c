/**
 * \file tft_ssd1963_50_1.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 * Controller:	SSD1963
 * Size:		5.0" (old version, glass A3271A-G)
 * Resolution:	800 x 480
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
#include "tft_ssd1963_50_1.h"


void ssd1963_50_1_address_set(unsigned int x1, unsigned int y1, unsigned int x2,
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


// Scales touch input, handles 180° rotation
void ssd1963_50_1_convert_touch_coordinates (void) {

	ly= (TP_X - SSD1963_50_1_Y_OFFSET) / SSD1963_50_1_Y_OFFSET_FACT;
	if (!lcd_rotation)
		ly = get_max_y() - ly;

	lx= (TP_Y - SSD1963_50_1_X_OFFSET) / SSD1963_50_1_X_OFFSET_FACT;
	if (!lcd_rotation)
		lx = get_max_x() - lx;

	if (lx < 0)
		lx = 0;
}


// Rotate by 180° via LCD controller
// rotation != 0 --> 180°
void ssd1963_50_1_rotate(uint8_t rotation)
{
    lcd_rotation = rotation;    // set global
    tft_set_pointer(SSD1963_set_address_mode);  //rotation
    if (rotation)
    	tft_write_byte(0x0000); // Upside down, 180°
    else
    	tft_write_byte(0x0003); // Back to normal, 0°
}


void ssd1963_50_1_init() {

	// set global information
	drv_convert_touch_coordinates = ssd1963_50_1_convert_touch_coordinates;
	drv_address_set = ssd1963_50_1_address_set;
    drv_lcd_rotate = ssd1963_50_1_rotate;
	// Return used resolution
	screen_max_x = T1_HDP;	// X
	screen_max_y = T1_VDP;	// Y

	// enable wait
	XMCRA |= (1<<SRW00) | (1<<SRW01);	// wait

	tft_set_pointer(SSD1963_set_pll_mn);	//PLL multiplier, set PLL clock to 120M
	tft_write_byte(0x0023);					//N=0x36 for 6.5M, 0x23 for 10M crystal
	tft_write_byte(0x0002);
	tft_write_byte(0x0004);
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
	tft_write_byte(0x0003);
	tft_write_byte(0x00ff);
	tft_write_byte(0x00ff);

	tft_set_pointer(SSD1963_set_lcd_mode);	//LCD SPECIFICATION
	tft_write_byte(0x0027);
	tft_write_byte(0x0000);
	tft_write_byte((T1_HDP>>8)&0X00FF);		//Set HDP
	tft_write_byte(T1_HDP&0X00FF);
	tft_write_byte((T1_VDP>>8)&0X00FF);		//Set VDP
	tft_write_byte(T1_VDP&0X00FF);
	tft_write_byte(0x0000);

	tft_set_pointer(SSD1963_set_hori_period);//HSYNC
	tft_write_byte((T1_HT>>8)&0X00FF);		//Set HT
	tft_write_byte(T1_HT&0X00FF);
	tft_write_byte((T1_HPS>>8)&0X00FF);		//Set HPS
	tft_write_byte(T1_HPS&0X00FF);
	tft_write_byte(T1_HPW);					//Set HPW
	tft_write_byte((T1_LPS>>8)&0X00FF);		//Set HPS
	tft_write_byte(T1_LPS&0X00FF);
	tft_write_byte(0x0000);

	tft_set_pointer(SSD1963_set_vert_period);//VSYNC
	tft_write_byte((T1_VT>>8)&0X00FF);		//Set VT
	tft_write_byte(T1_VT&0X00FF);
	tft_write_byte((T1_VPS>>8)&0X00FF);		//Set VPS
	tft_write_byte(T1_VPS&0X00FF);
	tft_write_byte(T1_VPW);					//Set VPW
	tft_write_byte((T1_FPS>>8)&0X00FF);		//Set FPS
	tft_write_byte(T1_FPS&0X00FF);

	tft_set_pointer(SSD1963_set_gpio_value);
	tft_write_byte(0x000F);					//GPIO[3:0] out 1

	tft_set_pointer(SSD1963_set_gpio_conf);
	tft_write_byte(0x0007);					//GPIO3=input, GPIO[2:0]=output
	tft_write_byte(0x0001);					//GPIO0 normal

	tft_set_pointer(SSD1963_set_address_mode);//rotation
	tft_write_byte(0x0003);
	//tft_write_byte(0x0000);

	tft_set_pointer(SSD1963_set_pixel_data_interface);//pixel data interface
	tft_write_byte(0x0003);

	NutDelay(5);

	tft_set_pointer(SSD1963_set_display_on); //display on

	tft_set_pointer(SSD1963_set_dbc_conf);
	tft_write_byte(0x000d);
}
