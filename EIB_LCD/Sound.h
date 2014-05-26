/** \file Sound.h
 *  \brief Constants and definitions for the sound output
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _SOUND_H_
#define _SOUND_H_

#include "System.h"

#define SOUND_MUTE_DDR		DDRB
#define	SOUND_MUTE_PORT		PORTB
#define SOUND_MUTE_BIT		0
#define SOUND_OUT_DDR		DDRE
#define SOUND_OUT_BIT		3

#define SOUND_INIT_MUTE		cbi (SOUND_MUTE_PORT, SOUND_MUTE_BIT); sbi (SOUND_MUTE_DDR, SOUND_MUTE_BIT);cbi (SOUND_OUT_DDR, SOUND_OUT_BIT);
#define SOUND_MUTE_ON		cbi (SOUND_MUTE_PORT, SOUND_MUTE_BIT); cbi (SOUND_OUT_DDR, SOUND_OUT_BIT);
#define SOUND_MUTE_OFF		sbi (SOUND_OUT_DDR, SOUND_OUT_BIT); sbi (SOUND_MUTE_PORT, SOUND_MUTE_BIT); 

// time in ms for beep
#define SOUND_BEEP_TIME			100
// interrupt handler for timer
//#define SOUND_PWM_INT		SIG_OVERFLOW3
#define SOUND_PWM_INT		TIMER3_OVF_vect
#define SOUND_VALUE_REG		OCR3A

// token marking the end of an audio clip in Flash
#define SOUND_END_TOKEN		0x7fff

typedef struct __attribute__ ((packed)) {
uint32_t	offset;
} _SOUND_DESCRIPTOR_t;


void sound_init (void);
void sound_beep_on (uint8_t);
void sound_silent (void);
void set_sound_table_start_address (uint32_t);
void sound_play_clip (uint16_t, uint8_t);
void sound_terminate_repetitions (void);

#endif // _SOUND_H_
