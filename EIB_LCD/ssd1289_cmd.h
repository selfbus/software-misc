/**
 * \file ssd1289_cmd.h
 *
 * \brief Constants for the SSD1289 LCD controller device
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef SSD1289_CMD_DEFINE_
#define SSD1289_CMD_DEFINE_

#define SSD1289_OSCSTART    0x00 /* Oscillation Start */
#define SSD1289_OUTCTRL     0x01 /* Driver output control */
#define SSD1289_ACCTRL      0x02 /* LCD drive AC control */
#define SSD1289_PWRCTRL1    0x03 /* Power control 1 */
#define SSD1289_CMP1        0x05 /* Compare register 1 */
#define SSD1289_CMP2        0x06 /* Compare register 2 */
#define SSD1289_DSPCTRL     0x07 /* Display control */
#define SSD1289_FCYCCTRL    0x0b /* Frame cycle control */
#define SSD1289_PWRCTRL2    0x0c /* Power control 2 */
#define SSD1289_PWRCTRL3    0x0d /* Power control 3 */
#define SSD1289_PWRCTRL4    0x0e /* Power control 4 */
#define SSD1289_GSTART      0x0f /* Gate scan start position */
#define SSD1289_SLEEP       0x10 /* Sleep mode */
#define SSD1289_ENTRY       0x11 /* Entry mode */
#define SSD1289_OPT3        0x12 /* Optimize Access Speed 3 */
#define SSD1289_GIFCTRL     0x15 /* Generic Interface Control */
#define SSD1289_HPORCH      0x16 /* Horizontal Porch */
#define SSD1289_VPORCH      0x17 /* Vertical Porch */
#define SSD1289_PWRCTRL5    0x1e /* Power control 5 */
#define SSD1289_DATA        0x22 /* RAM data/write data */
#define SSD1289_WRMASK1     0x23 /* RAM write data mask 1 */
#define SSD1289_WRMASK2     0x24 /* RAM write data mask 2 */
#define SSD1289_FFREQ       0x25 /* Frame Frequency */
#define SSD1289_VCOMOTP1    0x28 /* VCOM OTP */
#define SSD1289_OPT1        0x28 /* Optimize Access Speed 1 */
#define SSD1289_VCOMOTP2    0x29 /* VCOM OTP */
#define SSD1289_OPT2        0x2f /* Optimize Access Speed 2 */
#define SSD1289_GAMMA1      0x30 /* Gamma control 1 */
#define SSD1289_GAMMA2      0x31 /* Gamma control 2 */
#define SSD1289_GAMMA3      0x32 /* Gamma control 3 */
#define SSD1289_GAMMA4      0x33 /* Gamma control 4 */
#define SSD1289_GAMMA5      0x34 /* Gamma control 5 */
#define SSD1289_GAMMA6      0x35 /* Gamma control 6 */
#define SSD1289_GAMMA7      0x36 /* Gamma control 7 */
#define SSD1289_GAMMA8      0x37 /* Gamma control 8 */
#define SSD1289_GAMMA9      0x3a /* Gamma control 9 */
#define SSD1289_GAMMA10     0x3b /* Gamma control 10 */
#define SSD1289_VSCROLL1    0x41 /* Vertical scroll control 1 */
#define SSD1289_VSCROLL2    0x42 /* Vertical scroll control 2 */
#define SSD1289_HADDR       0x44 /* Horizontal RAM address position */
#define SSD1289_VSTART      0x45 /* Vertical RAM address start position */
#define SSD1289_VEND        0x46 /* Vertical RAM address end position */
#define SSD1289_W1START     0x48 /* First window start */
#define SSD1289_W1END       0x49 /* First window end */
#define SSD1289_W2START     0x4a /* Second window start */
#define SSD1289_W2END       0x4b /* Second window end */
#define SSD1289_XADDR       0x4e /* Set GDDRAM X address counter */
#define SSD1289_YADDR       0x4f /* Set GDDRAM Y address counter */

#endif
