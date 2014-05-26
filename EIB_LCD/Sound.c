/** \file ScreenCtrl.c
 *  \brief Functions for sound output
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- play sound samples from Nand Flash
 *	- play system beep
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "Sound.h"
#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/event.h>

//static HANDLE sound_silent_event;
static volatile uint16_t pwm_value;
static volatile uint16_t pwm_duration;

// start address of sound descriptor table in Flash
static uint32_t 	sound_table_start_address;
static volatile uint8_t		sound_sector;
static volatile uint16_t	sound_offset;
static volatile uint8_t		sound_sector_restart;
static volatile uint16_t	sound_offset_restart;
static volatile uint8_t		sound_clip_playing;
static volatile uint8_t		ampl_i;
static uint16_t ampl[] = { 256, 306, 256, 206 };
static volatile uint8_t		sound_repetitions;

// plays sound with ID "sound_clip". Special values:
// 0xfffe: no sound
// 0xffff: beep sound
void sound_play_clip (uint16_t sound_clip, uint8_t repetitions) {

uint32_t	ofs;
uint32_t	clip;

	// check, if sound should be played
	if (sound_clip == 0xfffe) {
		return;
	}
	// check, if beep or clip should be played
	if (sound_clip == 0xffff) {
		sound_beep_on (repetitions);
		return;
	}

	// get Flash start address of clip
	// get descriptor of picture i
	clip = sizeof (_SOUND_DESCRIPTOR_t) * sound_clip;
	clip += sound_table_start_address;

	// read sound properties from Flash
	ofs = read_flash_abs (clip +2);
	ofs = (ofs << 16) | read_flash_abs (clip);
	ofs += sound_table_start_address;

	NutEnterCritical();

	// no Beep at the same time!
	pwm_duration = 0;
	// calculate first sample
	sound_sector = FLASH_GET_SECTOR(ofs);
	sound_sector_restart = sound_sector;
	sound_offset = FLASH_GET_OFFSET(ofs) + FLASH_BASE_ADDRESS;
	sound_offset_restart = sound_offset;
	// setup sound value for 0%
	SOUND_VALUE_REG = 0x100;

	// enable sound timer
	sound_clip_playing = 1;
	// set OC3A on timer clear, clear on compare match
	TCCR3A |= (1 << COM3A1) | (1 << COM3A0);
	// 9 bit fast PWM mode
	TCCR3A |= (1 << WGM31);
	TCCR3A &= 0xff ^ (1 << WGM30);
	// 8 bit fast PWM mode
	TCCR3B |= (1 << WGM32);
	TCCR3B &= 0xff ^ (1 << WGM33);
	// clear overflow interrupt
	ETIFR |= (1 << TOV3);

	// unmask overflow interrupt
	ETIMSK |= (1 << TOIE3);
	// clear timer counter
	TCNT3 = 0;
	// start timer with 8MHz
	TCCR3B &= 0xff ^ ((1 << CS31) | (1 << CS32));
	TCCR3B |= (1 << CS30);
	// enable AMP
	SOUND_MUTE_OFF;

	sound_repetitions = repetitions;
	
	NutExitCritical();
	// setup sound value for 0%
	SOUND_VALUE_REG = 0x100;

}

// let the speaker beep by timer PWM output
void sound_beep_on (uint8_t repetitions) {

//	NutEnterCritical();
	// start PWM timer
	pwm_duration = 300 * (1+repetitions);
	// disable sound timer
	sound_clip_playing = 0;
	// set OC3A on timer clear, clear on compare match
	TCCR3A |= (1 << COM3A1) | (1 << COM3A0);
	// 10 bit fast PWM mode
	// 9 bit fast PWM mode
	TCCR3A |= (1 << WGM31);
	TCCR3A &= 0xff ^ (1 << WGM30);
	// 8 bit fast PWM mode
	TCCR3B |= (1 << WGM32);
	TCCR3B &= 0xff ^ (1 << WGM33);
	// clear timer counter
	TCNT3 = 0;
	// array index of amplitude values
	ampl_i = 0;
	SOUND_VALUE_REG = 0x100;
	// clear overflow interrupt
	ETIFR |= (1 << TOV3);

	NutEnterCritical();
	// unmask overflow interrupt
	ETIMSK |= (1 << TOIE3);
	NutExitCritical(); 
	// start timer with 8MHz
	TCCR3B &= 0xff ^ ((1 << CS31) | (1 << CS32));
	TCCR3B |= (1 << CS30);
	// enable AMP
	SOUND_MUTE_OFF; 
	// reload double buffer
	SOUND_VALUE_REG = 0x100;
}

// disables sound output
void sound_silent () {
	// disable AMP
	SOUND_MUTE_ON;
	// stop PWM timer
	TCCR3B = 0x00;
	// mark sound to be off
	pwm_value = 0; 
	pwm_duration = 0;
	sound_clip_playing = 0;
	sound_repetitions = 0;
}

// stops repetition of warning sounds
void sound_terminate_repetitions () {
	sound_repetitions = 0;
}


/*! \brief Timer Interrupt
 *         Indicates next PWM timer cycle
 *
 *  After each PWM cycle this interrupt is triggered. It should either reload the next PWM value 
 *  into the output compare register or stop the PWM output.
 */
ISR(SOUND_PWM_INT) /* vormals: SIGNAL(siglabel) dabei Vectorname != siglabel ! */
{
uint16_t next_value;
	// is it standard Beep or do we play a clip?
	if (pwm_duration) {
		// Beep
		if (!pwm_duration--) {
			sound_silent ();
		}
		else {
			SOUND_VALUE_REG = ampl[ampl_i++];
			if (ampl_i >= (sizeof (ampl) << 1))
				ampl_i = 0; 
		}
	}
	else if (sound_clip_playing) {
		// Clip
		// get next sound value
		next_value = read_flash_int(sound_sector, sound_offset);
		// check, if end token is read
		if (next_value == SOUND_END_TOKEN) {

			if (sound_repetitions) {
				sound_repetitions--;
				sound_sector = sound_sector_restart;
				sound_offset = sound_offset_restart;
				next_value = read_flash_int(sound_sector, sound_offset);
			}
			else {
				sound_silent ();
				return;
			}
		}
		// setup sound value
		SOUND_VALUE_REG = next_value;
		// recalc offset and sector
		if (!++sound_offset) {
			// overflow
			sound_offset = FLASH_BASE_ADDRESS;
			sound_sector++;
		}
	}
	// for safety
	else sound_silent ();
}

void set_sound_table_start_address (uint32_t s) {

	sound_table_start_address = s;

}

void sound_init () {

	SOUND_INIT_MUTE
	// init values for BEEP
	sound_silent ();
}



