/**
 * \file ssd1963_cmd.h
 *
 * \brief Constants for the SSD1963 LCD controller device
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef SSD1963_CMD_DEFINE_
#define SSD1963_CMD_DEFINE_

#define SSD1963_nop                 0x00 // No operation
#define SSD1963_soft_reset          0x01 // Software Reset
#define SSD1963_get_power_mode      0x0A // Get the current power mode
#define SSD1963_get_address_mode    0x0B // Get the frame memory to the display panel read order
//#define SSD1963_get_pixel_format 0x0C // Get the current pixel format
#define SSD1963_get_display_mode    0x0D // The display module returns the Display Signal Mode.
#define SSD1963_get_signal_mode     0x0E // Get the current display mode from the peripheral
#define SSD1963_enter_sleep_mode    0x10 // Turn off the panel. This command will pull low the GPIO0. If GPIO0 is configured as normal GPIO or LCD miscellaneous signal with command set_gpio_conf, this command will be ignored. 
#define SSD1963_exit_sleep_mode     0x11 // Turn on the panel. This command will pull high the GPIO0.  If GPIO0 is configured as normal GPIO or LCD  miscellaneous signal with command set_gpio_conf, this  command will be ignored. 
#define SSD1963_enter_partial_mode  0x12 // Part of the display area is used for image display.
#define SSD1963_enter_normal_mode   0x13 // The whole display area is used for image display.
#define SSD1963_exit_invert_mode    0x20 // Displayed image colors are not inverted.
#define SSD1963_enter_invert_mode   0x21 // Displayed image colors are inverted.
#define SSD1963_set_gamma_curve     0x26 // Selects the gamma curve used by the display device.
#define SSD1963_set_display_off     0x28 // Blanks the display device
#define SSD1963_set_display_on      0x29 // Show the image on the display device
#define SSD1963_set_column_address  0x2A // Set the column extent
#define SSD1963_set_page_address    0x2B // Set the page extent
#define SSD1963_write_memory_start  0x2C // Transfer image information from the host processor interface  to the peripheral starting at the location provided by  set_column_address and set_page_address 
#define SSD1963_read_memory_start   0x2E // Transfer image data from the peripheral to the host processor  interface starting at the location provided by  set_column_address and set_page_address 
#define SSD1963_set_partial_area    0x30 // Defines the partial display area on the display device
#define SSD1963_set_scroll_area     0x33 // Defines the vertical scrolling and fixed area on display area
#define SSD1963_set_tear_off        0x34 // Synchronization information is not sent from the display  module to the host processor 
#define SSD1963_set_tear_on         0x35 // Synchronization information is sent from the display module  to the host processor at the start of VFP
#define SSD1963_set_address_mode    0x36 // Set the read order from frame buffer to the display panel
#define SSD1963_set_scroll_start    0x37 // Defines the vertical scrolling starting point
#define SSD1963_exit_idle_mode      0x38 // Full color depth is used for the display panel
#define SSD1963_enter_idle_mode     0x39 // Reduce color depth is used on the display panel.
#define SSD1963_set_pixel_format    0x3A // Defines how many bits per pixel are used in the interface
#define SSD1963_write_memory_continue   0x3C // Transfer image information from the host processor interface  to the peripheral from the last written location 
#define SSD1963_read_memory_continue    0x3E // Read image data from the peripheral continuing after the last  read_memory_continue or read_memory_start 
#define SSD1963_set_tear_scanline   0x44 // Synchronization information is sent from the display module  to the host processor when the display device refresh reaches  the provided scanline 
#define SSD1963_get_scanline        0x45 // Get the current scan line
#define SSD1963_read_ddb            0xA1 // Read the DDB from the provided location
#define SSD1963_set_lcd_mode        0xB0 // Set the LCD panel mode (RGB TFT or TTL)
#define SSD1963_get_lcd_mode        0xB1 // Get the current LCD panel mode, pad strength and resolution
#define SSD1963_set_hori_period     0xB4 // Set front porch
#define SSD1963_get_hori_period     0xB5 // Get current front porch settings
#define SSD1963_set_vert_period     0xB6 // Set the vertical blanking interval between last scan line and  next LFRAME pulse
#define SSD1963_get_vert_period     0xB7 // Set the vertical blanking interval between last scan line and  next LFRAME pulse
#define SSD1963_set_gpio_conf       0xB8 // Set the GPIO configuration. If the GPIO is not used for LCD,  set the direction. Otherwise, they are toggled with LCD  signals.
#define SSD1963_get_gpio_conf       0xB9 //  Get the current GPIO configuration 
#define SSD1963_set_gpio_value      0xBA //  Set GPIO value for GPIO configured as output 
#define SSD1963_get_gpio_status     0xBB // Read current GPIO status. If the individual GPIO was  configured as input, the value is the status of the  corresponding pin. Otherwise, it is the programmed value. 
#define SSD1963_set_post_proc       0xBC // Set the image post processor
#define SSD1963_get_post_proc       0xBD // Set the image post processor
#define SSD1963_set_pwm_conf        0xBE // Set the image post processor
#define SSD1963_get_pwm_conf        0xBF // Set the image post processor
#define SSD1963_set_lcd_gen0        0xC0 // Set the rise, fall, period and toggling properties of LCD signal  generator 0
#define SSD1963_get_lcd_gen0        0xC1 // Get the current settings of LCD signal generator 0
#define SSD1963_set_lcd_gen1        0xC2 // Set the rise, fall, period and toggling properties of LCD signal  generator 1
#define SSD1963_get_lcd_gen1        0xC3 // Get the current settings of LCD signal generator 1
#define SSD1963_set_lcd_gen2        0xC4 // Set the rise, fall, period and toggling properties of LCD signal  generator 2
#define SSD1963_get_lcd_gen2        0xC5 // Get the current settings of LCD signal generator 2
#define SSD1963_set_lcd_gen3        0xC6 // Set the rise, fall, period and toggling properties of LCD signal  generator 3
#define SSD1963_get_lcd_gen3        0xC7 // Get the current settings of LCD signal generator 3
#define SSD1963_set_gpio0_rop       0xC8 // Set the GPIO0 with respect to the LCD signal generators  using ROP3 operation. No effect if the GPIO0 is configured  as general GPIO.
#define SSD1963_get_gpio0_rop       0xC9 // Get the GPIO0 properties with respect to the LCD signal  generators. 
#define SSD1963_set_gpio1_rop       0xCA // Set the GPIO1 with respect to the LCD signal generators  using ROP3 operation. No effect if the GPIO1 is configured  as general GPIO.
#define SSD1963_get_gpio1_rop       0xCB // Get the GPIO1 properties with respect to the LCD signal  generators. 
#define SSD1963_set_gpio2_rop       0xCC // Set the GPIO2 with respect to the LCD signal generators  using ROP3 operation. No effect if the GPIO2 is configured  as general GPIO.
#define SSD1963_get_gpio2_rop       0xCD // Get the GPIO2 properties with respect to the LCD signal  generators. 
#define SSD1963_set_gpio3_rop       0xCE // Set the GPIO3 with respect to the LCD signal generators  using ROP3 operation. No effect if the GPIO3 is configured  as general GPIO.
#define SSD1963_get_gpio3_rop       0xCF // Get the GPIO3 properties with respect to the LCD signal  generators.
#define SSD1963_set_dbc_conf        0xD0 // Set the dynamic back light configuration
#define SSD1963_get_dbc_conf        0xD1 // Get the current dynamic back light configuration
#define SSD1963_set_dbc_th          0xD4 // Set the threshold for each level of power saving
#define SSD1963_get_dbc_th          0xD5 // Get the threshold for each level of power saving
#define SSD1963_set_pll             0xE0 // Start the PLL. Before the start, the system was operated with  the crystal oscillator or clock input
#define SSD1963_set_pll_mn          0xE2 // Set the PLL
#define SSD1963_get_pll_mn          0xE3 // Get the PLL settings
#define SSD1963_get_pll_status      0xE4 // Get the current PLL status
#define SSD1963_set_deep_sleep      0xE5 // Set deep sleep mode
#define SSD1963_set_lshift_freq     0xE6 // Set the LSHIFT (pixel clock) frequency
#define SSD1963_get_lshift_freq     0xE7 // Get current LSHIFT (pixel clock) frequency setting
#define SSD1963_set_pixel_data_interface    0xF0 // Set the pixel data format of the parallel host processor  interface 
#define SSD1963_get_pixel_data_interface    0xF1 // Get the current pixel data format settings

//SSL[15:8] : Supplier ID of Solomon Systech Limited high byte, always 01h (POR = 00000001) 
#define SSD1963_SSL_H   0x01
//SSL[7:0] : Supplier ID of Solomon Systech Limited low byte, always 57h (POR = 010101110) 
#define SSD1963_SSL_L   0x57
//PROD[7:0] : Product ID, always 61h (POR = 01100001) 
#define	SSD1963_PROD    0x61
//REV[2:0] : Revision code, always 01h (POR = 001) 
#define	SSD1963_REV     0x01
//Exit code, always FFh (POR = 11111111) 			
#define	SSD1963_EXIT    0xff

#endif
