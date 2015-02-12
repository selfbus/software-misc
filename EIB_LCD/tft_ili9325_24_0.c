/**
 * \file tft_ili9325_24_0.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 * Controller:	ILI9325
 * Size:		2.4"
 * Resolution:	320 x 240
 * Order codes:	170709357972, 270569308217
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_ili9325_24_0.h"


void ili9325_24_0_address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

		main_W_com_data(TS_INS_START_ADX, x1);
		main_W_com_data(TS_INS_END_ADX, x2);
		main_W_com_data(TS_INS_GRAM_ADX, x1);

		main_W_com_data(TS_INS_START_ADY, y1);
		main_W_com_data(TS_INS_END_ADY, y2);
		main_W_com_data(TS_INS_GRAM_ADY, y1);
		tft_set_pointer(0x22);
}


void ili9325_24_0_convert_touch_coordinates (void) {

	ly = (TP_Y - ILI9325_Y_OFFSET) / ILI9325_Y_OFFSET_FACT;

	//The 2.4" TFT using ILI9325 has mirrored X-Coordinates!
	if (!invert_touch_x) {
		if (TP_X >= 380) {
			lx = (3910 - TP_X) / 11;
			if (lx < 0)
				lx = 0;
		} else
			lx = (TP_X - 391) / 11;

	} else
		lx = (TP_X - 350) / 11;
}

void ili9325_24_0_rotate(uint8_t rotation)
{
    lcd_rotation = rotation;    // set global

//FIXME: add code for display rotation here

}

void ili9325_24_0_init() {

	// set global information
	drv_convert_touch_coordinates = ili9325_24_0_convert_touch_coordinates;
	drv_address_set = ili9325_24_0_address_set;
	drv_lcd_rotate = ili9325_24_0_rotate;
	// Return used resolution
	screen_max_x = 319;	// X
	screen_max_y = 239;	// Y

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
