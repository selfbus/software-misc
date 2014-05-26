/** \file rc5_io.c
 *  \brief Basic IO functions for RC5 reception
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- poll GPIO input pin and assemble IR message
 *	- decode received IR message
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "rc5_io.h"
#include "io.h"

uint8_t	rc5_qcnt;	// bit quanta counter for RC5 reception
uint8_t	rc5_last;	// last received bit state
uint8_t	rc5_bcnt;	// bit counter for RC5 message reception
uint8_t rc5_msg[4];	// RC5 reception buffer
uint8_t	rc5_full;	// RC5 reception buffer is loaded with a complete message
uint8_t	rc5_a;		// RC5 address
uint8_t rc5_c;		// RC5 command
uint8_t rc5_t;		// RC5 toggle bit
uint8_t	rc5_enabled;// RC5 receiver is initialized

void init_rc5 () {

	if (rc5_enabled)
		return;	

	RC5_DDR &= 0xff ^ RC5_BIT_MSK;
	RC5_PORT |= RC5_BIT_MSK;
	// init variables of rc5 driver
	rc5_bcnt = 0;
	rc5_full = 0;
	// enable interrupt of backlight PWM timer. This gives an interrupt every 256us
	TIMSK |= (1 << TOIE2);
	// set speed of PWM timer
	TFT_BACKLIGHT_FAST_PWM;

	rc5_enabled = 1;

}

// this function returns 0, if b contains an alternating bit pattern.
uint8_t rc5_check_byte (uint8_t b) {

	b ^= (b >> 1);
	b &= 0x55;

	return (b != 0x55);

}

// this function moves d7, d5, d3 and d1 into d3, d2, d1, d0.
uint8_t rc5_compact_byte (uint8_t b) {

int i;
uint8_t r = 0;

	for ( i = 0; i < 4; i++) {

		r = r << 1;
		if (b & 0x80)
			r |= 0x01;
		b = b << 2;

	}
	
	return r;

}

uint8_t rc5_decode_message (void) {


	// check start bit
	if ((rc5_msg[0] & 0xC0) != 0x80) {
		rc5_full = 0;
		return 0;
	}
	// check alternating 1 and 0 per bit frame
	if (rc5_check_byte(rc5_msg[0]) || 
		rc5_check_byte(rc5_msg[1]) ||
		rc5_check_byte(rc5_msg[2]) ||
		rc5_check_byte(rc5_msg[3] | 0x05)) {
		rc5_full = 0;
		return 0;
	}

	// now the bits are alternating

	// decode address
	rc5_a = rc5_compact_byte (rc5_msg[1]);
	if (rc5_msg[0] & 0x2)
		rc5_a |= 0x10;

	// decode toggle bit
	rc5_t = (rc5_msg[0] >> 3) & 0x01;

	// decode command
	rc5_c = rc5_compact_byte (rc5_msg[2]) << 2;
	rc5_c |= rc5_compact_byte (rc5_msg[3]) >> 2;
	if (!(rc5_msg[0] & 0x20))
		rc5_c |= 0x40;

	rc5_full = 0;
	return 1;

}

void terminate_rc5 (void) {

	// disable interrupt of backlight PWM timer.
	TIMSK &= (0xff ^ (1 << TOIE2));
	TFT_BACKLIGHT_SLOW_PWM;
	RC5_PORT &= 0xff ^ RC5_BIT_MSK;
	rc5_enabled = 0;	

}

void rc5_store_half_bit (uint8_t b) {

	if (b)
		rc5_msg [rc5_bcnt >> 3] |= 0x80 >> (rc5_bcnt&7);
	rc5_bcnt++;

	if (rc5_bcnt > 27) {
		rc5_full = 1;
		rc5_bcnt = 0;
		rc5_qcnt = 0;
	}
}

/*! \brief Timer Interrupt
 *         Indicates next PWM timer cycle
 *
 */
//ISR(SIG_OVERFLOW2)
ISR(TIMER2_OVF_vect )
{

	uint8_t	ns;
	// get current input level of RC5 receiver input pin
	ns = RC5_PIN & RC5_BIT_MSK;
	if (!ns || rc5_bcnt) {
		
		// either we received a start bit or a message reception is ongoing
		if (ns == rc5_last) {
			// no toggle, increment qcnt;
			rc5_qcnt++;
			if ((rc5_qcnt > 8) && (rc5_bcnt == 27) && (ns)) {
				// this is end of a frame
				rc5_store_half_bit (ns);
			}
			else if (rc5_qcnt > 8) {
				// this is a reception error
				rc5_qcnt = 0;
				rc5_bcnt = 0;
			}
		}
		else {
			// check for start condition of a message
			if (!rc5_qcnt && !rc5_bcnt && !rc5_full) {
				// start condition is met
				rc5_qcnt = 1;
				rc5_bcnt = 1;
				rc5_msg[0] = 0x80;
				rc5_msg[1] = 0;
				rc5_msg[2] = 0;
				rc5_msg[3] = 0;
			}
			// received a toggle, see what qcnt is:
			else if ((rc5_qcnt < 3) || (rc5_qcnt > 8)) {
				// it must be a reception error
				rc5_qcnt = 0;
				rc5_bcnt = 0;
			}
			else {
				rc5_store_half_bit (rc5_last);
				if (rc5_qcnt > 5)
					rc5_store_half_bit (rc5_last);
				if (rc5_bcnt)
					rc5_qcnt = 1;
			}
		}
	}
	
	rc5_last = ns;

}
