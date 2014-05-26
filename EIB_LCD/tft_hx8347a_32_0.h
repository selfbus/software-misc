/**
 * \file tft_hx8347a_32_0.h
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
#ifndef TFT_HX8347A_32_0_H_
#define TFT_HX8347A_32_0_H_

#include "tft_io.h"

// The calibration values are always related to ly and lx
// Touch offset values for 3.2" LCD
#define HX8347A_Y_OFFSET		200
#define HX8347A_Y_OFFSET_FACT	15
#define HX8347A_X_OFFSET		350
#define HX8347A_X_OFFSET_FACT	11

void hx8347a_32_0_init(void);

#endif //TFT_HX8347A_32_0_H_
