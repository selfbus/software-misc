/**
 * \file tft_ssd1289_32_0.h
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
#ifndef TFT_SSD1289_32_0_H_
#define TFT_SSD1289_32_0_H_

#include "tft_io.h"
#include "ssd1289_cmd.h"

// The calibration values are always related to ly and lx
// Touch offset values for 3.2" LCD
#define SSD1289_Y_OFFSET		200
#define SSD1289_Y_OFFSET_FACT	15
#define SSD1289_X_OFFSET		350
#define SSD1289_X_OFFSET_FACT	11


void ssd1289_32_0_init(void);

#endif //TFT_SSD1289_32_0_H_
