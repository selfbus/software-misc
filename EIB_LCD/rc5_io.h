/** \file rc5_io.h
 *  \brief Constants and definitions for the RC5 reception
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef RC5_IO_H_
#define RC5_IO_H_

#include "System.h"

#define RC5_DDR			DDRE
#define RC5_PORT		PORTE
#define	RC5_PIN			PINE
#ifdef LCD_DEBUG
#define	RC5_BIT		2
#else
#define	RC5_BIT		1
#endif
#define	RC5_BIT_MSK	(1 << RC5_BIT)

// constants for RC5 operation
#define RC5_RELEASED_LONG		4
#define RC5_RELEASED_SHORT		2
#define RC5_PRESSED_LONG		3
#define RC5_PRESSED_AUTO		5
#define RC5_PRESSED_NEW			1

// public variables
extern uint8_t	rc5_full;
extern uint8_t	rc5_msg[4];
extern uint8_t	rc5_a;		// RC5 address
extern uint8_t rc5_c;		// RC5 command
extern uint8_t rc5_t;		// RC5 toggle bit

// public functions
void init_rc5 (void);
void terminate_rc5 (void);
uint8_t rc5_decode_message (void);


#endif //RC5_IO_H_
