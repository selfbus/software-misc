/**
 * \file tft_ili9325_24_0.h
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
#ifndef TFT_ILI9325_24_0_H_
#define TFT_ILI9325_24_0_H_

#include "tft_io.h"

// The calibration values are always related to ly and lx
// Touch offset values for 3.2" LCD
#define ILI9325_Y_OFFSET		200
#define ILI9325_Y_OFFSET_FACT	15
#define ILI9325_X_OFFSET		350
#define ILI9325_X_OFFSET_FACT	11

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

#define TS_INS_GRAM_ADX				TS_INS_GRAM_VER_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_HOR_AD
#define TS_INS_START_ADX   			TS_INS_VER_START_AD
#define TS_INS_END_ADX   			TS_INS_VER_END_AD
#define TS_INS_START_ADY   			TS_INS_HOR_START_AD
#define TS_INS_END_ADY   			TS_INS_HOR_END_AD

void ili9325_24_0_init(void);

#endif //TFT_ILI9325_24_0_H_
