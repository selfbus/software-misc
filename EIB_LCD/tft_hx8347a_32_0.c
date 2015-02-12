/**
 * \file tft_hx8347a_32_0.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 * Controller:	HX8347A
 * Size:		3.2"
 * Resolution:	320 x 240
 * Order codes:	200475566068 (old version, out of stock)
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_hx8347a_32_0.h"


void hx8347a_32_0_address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

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


void hx8347a_32_0_convert_touch_coordinates (void) {

	ly = (TP_Y - HX8347A_Y_OFFSET) / HX8347A_Y_OFFSET_FACT;

	lx = (TP_X - HX8347A_X_OFFSET) / HX8347A_X_OFFSET_FACT;

}

void hx8347a_32_0_rotate(uint8_t rotation)
{
    lcd_rotation = rotation;    // set global

//FIXME: add code for display rotation here

}

void hx8347a_32_0_init() {

	// set global information
	drv_convert_touch_coordinates = hx8347a_32_0_convert_touch_coordinates;
	drv_address_set = hx8347a_32_0_address_set;
	drv_lcd_rotate = hx8347a_32_0_rotate;
	// Return used resolution
	screen_max_x = 319;	// X
	screen_max_y = 239;	// Y

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
