/**
 * \file tft_ssd1289_32_0.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 * Controller:	SSD1289
 * Size:		3.2"
 * Resolution:	320 x 240
 * Order codes:	200475566068
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_ssd1289_32_0.h"


void ssd1289_32_0_address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

	main_W_com_data(0x44, (y2 << 8) + y1);
	main_W_com_data(0x45, x1);
	main_W_com_data(0x46, x2);
	main_W_com_data(0x4e, y1);
	main_W_com_data(0x4f, x1);

	tft_set_pointer(0x22);
}


void ssd1289_32_0_convert_touch_coordinates (void) {

	ly = (TP_Y - SSD1289_Y_OFFSET) / SSD1289_Y_OFFSET_FACT;

	if (invert_touch_x) {
		if (TP_X >= 380) {
			lx = (3910 - TP_X) / SSD1289_X_OFFSET_FACT;
			if (lx < 0)
				lx = 0;
		} else
			lx = (TP_X - 391) / SSD1289_X_OFFSET_FACT;

	} else
		lx = (TP_X - 350) / SSD1289_X_OFFSET_FACT;

}


void ssd1963_32_0_rotate(uint8_t rotation)
{
    if (rotation) main_W_com_data(SSD1289_OUTCTRL, 0x693F);   // Upside down, RL=1
    else main_W_com_data(SSD1289_OUTCTRL, 0x293F);          // Normal, RL=0    
}


void ssd1289_32_0_init() {

	// set global information
	drv_convert_touch_coordinates = ssd1289_32_0_convert_touch_coordinates;
	drv_address_set = ssd1289_32_0_address_set;
    drv_lcd_rotate = ssd1963_32_0_rotate;
	// Return used resolution
	screen_max_x = 319;	// X
	screen_max_y = 239;	// Y

	main_W_com_data(SSD1289_OSCSTART, 0x0001);
	NutDelay(2);
	main_W_com_data(SSD1289_PWRCTRL1, 0xA8A4);
	NutDelay(2);
	main_W_com_data(SSD1289_PWRCTRL2, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_PWRCTRL3, 0x080C);
	NutDelay(2);
	main_W_com_data(SSD1289_PWRCTRL4, 0x2B00);
	NutDelay(2);
	main_W_com_data(SSD1289_PWRCTRL5, 0x00B0);
	NutDelay(2);
    //if (rotate) main_W_com_data(SSD1289_OUTCTRL, 0x693F);   // Rotation, RL=1
    //else
    main_W_com_data(SSD1289_OUTCTRL, 0x293F);          // Normal, RL=0
	NutDelay(2); //320*240  0x6B3F
	main_W_com_data(SSD1289_ACCTRL, 0x0600);
	NutDelay(2);
	main_W_com_data(SSD1289_SLEEP, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_ENTRY, 0x6078);
	NutDelay(2); //0x4030
	main_W_com_data(SSD1289_CMP1, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_CMP2, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_HPORCH, 0xEF1C);
	NutDelay(2);
	main_W_com_data(SSD1289_VPORCH, 0x0003);
	NutDelay(2);
	main_W_com_data(SSD1289_DSPCTRL, 0x0233);
	NutDelay(2);
	main_W_com_data(SSD1289_FCYCCTRL, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_GSTART, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_VSCROLL1, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_VSCROLL2, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_W1START, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_W1END, 0x013F);
	NutDelay(2);
	main_W_com_data(SSD1289_W2START, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_W2END, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_HADDR, 0xEF00);
	NutDelay(2);
	main_W_com_data(SSD1289_VSTART, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_VEND, 0x013F);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA1, 0x0707);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA2, 0x0204);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA3, 0x0204);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA4, 0x0502);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA5, 0x0507);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA6, 0x0204);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA7, 0x0204);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA8, 0x0502);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA9, 0x0302);
	NutDelay(2);
	main_W_com_data(SSD1289_GAMMA10, 0x0302);
	NutDelay(2);
	main_W_com_data(SSD1289_WRMASK1, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_WRMASK2, 0x0000);
	NutDelay(2);
	main_W_com_data(SSD1289_FFREQ, 0x8000);
	NutDelay(2);
	main_W_com_data(SSD1289_YADDR, 0);
	main_W_com_data(SSD1289_XADDR, 0);
}
