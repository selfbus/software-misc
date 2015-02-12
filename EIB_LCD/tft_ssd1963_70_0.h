/**
 * \file tft_ssd1963_70_0.h
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
#ifndef TFT_SSD1963_70_0_H_
#define TFT_SSD1963_70_0_H_

#include "tft_io.h"
#include "ssd1963_cmd.h"

// Touch offset values for 7.0" LCD (Type 3)
// Measured x --> ly and Y_OFFSET
// Measured y --> lx and X_OFFSET
#define SSD1963_70_Y_OFFSET			360	//3800, 360
#define SSD1963_70_Y_OFFSET_FACT	7.2
#define SSD1963_70_X_OFFSET			130 //3900
#define SSD1963_70_X_OFFSET_FACT	4.7

#define		T3_HDP		799	//-->800
#define		T3_HT		1050
#define		T3_HPS		46
#define 	T3_LPS		210
#define 	T3_HPW		20

#define		T3_VDP		479	//-->480
#define 	T3_VT		525
#define		T3_VPS		23
#define		T3_FPS		22
#define		T3_VPW		10


void ssd1963_70_0_init(void);

#endif //TFT_SSD1963_70_0_H_
