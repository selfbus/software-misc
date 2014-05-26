/** \file EIB_LCD.c
 *  \brief Main file of the firmware containing the main() function. 
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	This module contains the main() control function of the LCD Module,
 *	which the Nut/OS init code calls at startup. The main() function
 *	never terminates. After setting up the hardware and initialisising
 *	the system it runns an endless loop.
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
  
#include "System.h"
#include "tft_io.h"

#include <dev/nplmmc.h>
#include <dev/sbi_mmc.h>
#include <fs/phatfs.h>
#include <dev/mmcard.h>

#include "TPUart.h"
#include "EIBLayers.h"

/*! global flag indicating the check result of the project data in the external Flash memory:
	0: Flash content is not yet checked, corrupted or has the wrong structure version
	1: Flash content is checked and ok
*/
uint8_t	flash_content_ok;

/*!
	Definition to obtain the version of the firmware image
*/
const bootldrinfo_t bootlodrinfo __attribute__ ((section (".bootldrinfo"))) = {DEVID, SWVERSIONMAJOR << 8 | SWVERSIONMINOR, 0x0000};



/*!
 * \brief Main application routine.
 *
 * Nut/OS automatically calls this function after initialization. 
 * main() calls the init functions of all software modules, which 
 * create new threads for their cyclic tasks. Finally, it runns
 * an endless loop never returning control to the caller.
 */
int main(void)
{
	/* reset some hardware configs to cope with Bootloader */
	init_hardware ();

	/* init the status LED on the LCD controller board */
	LED_INIT;

    /*
     * Initialize the uart device, if debugging is enabled. The user can use the IO pins shared with the UART signals 
	 * to attach his external hardware, so we should not enable the UART for released software.
     */
#if defined(LCD_DEBUG) || defined(HW_DEBUG) || defined(TOUCH_DEBUG)
uint32_t baud = 115200;
    NutRegisterDevice(&DEV_DEBUG, 0, 0);
    freopen(DEV_DEBUG_NAME, "w", stdout);
    _ioctl(_fileno(stdout), UART_SETSPEED, &baud);
	// FIXME: calculated frequency setting seems wrong!
	UBRR0L = 0x08;
    NutSleep(200);
    printf_P(PSTR("\n\nNut/OS %s "), NutVersionString());
	printf_P(PSTR("\nFirmware V%d.%d"), pgm_read_byte_far((char*)&bootlodrinfo.app_version +1), pgm_read_byte_far((char*)&bootlodrinfo.app_version));
	printf_P(PSTR("\nBuild %s with GCC %s"), __TIMESTAMP__, __VERSION__);
#endif
	/* set external memory i/f timing */
	XMCRB |= (1<<XMBK); // enable bus keepers
	XMCRA &= 0xff ^ ( (1<<SRL1) | (1<<SRL0) | (1<<SRW01) | (1<<SRW00) | (1<<SRW11) ); // no wait
	XMCRA |= (1<<SRL2) ; // divide memory at 0x8000
	MCUCR &= 0xff ^ ( (1<<SRW10) ); // no wait

	/* init tft controller */
	tft_init();
#ifdef LCD_DEBUG
    printf_P(PSTR("TFT initialized.\n"));
#endif

	/* from now onwards we can output status messages to the TFT. It helps for debugging in case the start procedure crashes. */
    printf_tft_P( TFT_COLOR_BLACK, TFT_COLOR_WHITE, PSTR("Nut/OS %s "), NutVersionString());
	/* setup the external Flash memory device */
	printf_tft_P( TFT_COLOR_GREEN, TFT_COLOR_WHITE, PSTR("Init NAND Flash"));
	init_nand_flash();
	/* read project information from external NAND Flash and configure the system accordingly. */
	init_system_from_flash ();
	/* start the EIB communication */
	init_eib_layers ();
	/* init sound functions */
	sound_init ();
	/* start sd card driver */
	init_sd_card ();
	/* start touch function */
	touch_init();
	/* init screen control functions */
	init_screen_control();

	/* prevent user of TFT modules without extra touch area from locking out */
	if (controller_type == 4) 
		create_system_info_screen ();
	else 
		/* show 1st page */
		set_page (0);

    /*
     * This is the main thread running with lowest priority
     */
    NutThreadSetPriority(250);
    for (;;) {
        NutSleep(MAIN_TIME_LOOP_SLEEP); // currently 30ms

		/* check TPUART status */
		eib_get_status();
		/* Caution against TX deadlocks */
		eib_check_tx_deadlock();

		/* tick countdown of screen lock timer */
		check_screen_lock();

		/* process timer event for listening elements independent of pages */
		lcd_listen_timer_event();
		/* process timer event for cyclic elements independent of pages */
		lcd_cyclic_process_event ();
		/* process page elements for cyclic functions on pages */
		process_cyclic_page_events ();

    }
	/* GCC likes to see a return here. Of course it has no meaning an is never executed. */
    return 0;
}
