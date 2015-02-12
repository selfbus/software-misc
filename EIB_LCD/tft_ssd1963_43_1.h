/**
 * \file tft_ssd1963_43_1.h
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
#ifndef TFT_SSD1963_43_1_H_
#define TFT_SSD1963_43_1_H_

#include "tft_io.h"
#include "ssd1963_cmd.h"

// Touch offset values for old 4.3" LCD
#define SSD1963_43_1_Y_OFFSET		340
#define SSD1963_43_1_Y_OFFSET_FACT	12.3
#define SSD1963_43_1_X_OFFSET		170
#define SSD1963_43_1_X_OFFSET_FACT	7.7

#define		T4_HDP		479	//-->480
#define		T4_HT		531
#define		T4_HPS		43
#define 	T4_LPS		8
#define 	T4_HPW		1

#define		T4_VDP		271	//-->272
#define 	T4_VT		288
#define		T4_VPS		12
#define		T4_FPS		4
#define		T4_VPW		10


void ssd1963_43_1_init(void);

#endif //TFT_SSD1963_43_1_H_
