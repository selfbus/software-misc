/**
 * \file tft_ssd1963_50_1.h
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
#ifndef TFT_SSD1963_50_1_H_
#define TFT_SSD1963_50_1_H_

#include "tft_io.h"
#include "ssd1963_cmd.h"

// Touch offset values for 5.0" LCD (Type 1)
// Measured x --> ly and Y_OFFSET
// Measured y --> lx and X_OFFSET
#define SSD1963_50_1_Y_OFFSET		190
#define SSD1963_50_1_Y_OFFSET_FACT	7.5
#define SSD1963_50_1_X_OFFSET		220 //3900
#define SSD1963_50_1_X_OFFSET_FACT	4.6	//4.7


#define	T1_HDP		799	//-->800
#define	T1_HT		928
#define	T1_HPS		46
#define T1_LPS		15
#define T1_HPW		48

#define	T1_VDP		479	//-->480
#define T1_VT		525
#define	T1_VPS		16
#define	T1_FPS		8
#define	T1_VPW		16


void ssd1963_50_1_init(void);

#endif //TFT_SSD1963_50_1_H_
