/**
 * \file tft_ssd1963_43_0.h
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
#ifndef TFT_SSD1963_43_0_H_
#define TFT_SSD1963_43_0_H_

#include "tft_io.h"
#include "ssd1963_cmd.h"

// Touch offset values for new 4.3" LCD (Type 2)
// Measured x --> ly and Y_OFFSET
// Measured y --> lx and X_OFFSET
#define SSD1963_43_0_Y_OFFSET		430		//340, 3540
#define SSD1963_43_0_Y_OFFSET_FACT	11.4	//12.3
#define SSD1963_43_0_X_OFFSET		250		//170
#define SSD1963_43_0_X_OFFSET_FACT	7.6		//7.7

#define		T2_HDP		479	//-->480
#define		T2_HT		531
#define		T2_HPS		43
#define 	T2_LPS		8
#define 	T2_HPW		10

#define		T2_VDP		271	//-->272
#define 	T2_VT		288
#define		T2_VPS		12
#define		T2_FPS		4
#define		T2_VPW		10


void ssd1963_43_0_init(void);

#endif //TFT_SSD1963_43_0_H_
