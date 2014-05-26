/**
 * \file tft_ssd1963_50_0.h
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
#ifndef TFT_SSD1963_50_0_H_
#define TFT_SSD1963_50_0_H_

#include "tft_io.h"
#include "ssd1963_cmd.h"

// Touch offset values for 5.0" LCD (Type 0)
// Measured x --> ly and Y_OFFSET
// Measured y --> lx and X_OFFSET
#define SSD1963_0_50_Y_OFFSET		200	//3700
#define SSD1963_0_50_Y_OFFSET_FACT	7.3
#define SSD1963_0_50_X_OFFSET		220 //3900
#define SSD1963_0_50_X_OFFSET_FACT	4.6	//4.7


#define	T0_HDP		799	//-->800
#define	T0_HT		1050
#define	T0_HPS		46
#define T0_LPS		15
#define T0_HPW		48

#define	T0_VDP		479	//-->480
#define T0_VT		525
#define	T0_VPS		16
#define	T0_FPS		8
#define	T0_VPW		16


void ssd1963_50_0_init(void);

#endif //TFT_SSD1963_50_0_H_
