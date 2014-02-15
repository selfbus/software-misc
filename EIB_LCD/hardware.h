/** \file hardware.h
 *  \brief Constants and definitions for EIB-LCD board hardware
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

//definitions for TPUART driver
#define EIB_UART	1
//#define EIB_VOLTAGE_CHANNEL	ADC0	/*!< ADC channel monitoring EIB voltage */
#define EIB_RESET_OUT_PORT	PORTD
#define EIB_RESET_OUT_DDR	DDRD
#define EIB_RESET_OUT_BIT	5
#define EIB_RESET_PIN		PIND
#define EIB_RESET_IN_BIT	4

// enable pull-up resistor of RxD0 pin
#define ENABLE_RXD0_PULL_UP sbi(PORTE, 0)


//define LED control
#define	LED_DDR		DDRF
#define	LED_PORT	PORTF
#define	LED_BIT		3
#define LED_INIT	sbi (LED_PORT, LED_BIT); sbi (LED_DDR, LED_BIT);
#define LED_ON		cbi (LED_PORT, LED_BIT);
#define LED_OFF		sbi (LED_PORT, LED_BIT);

//PE0 (1)", "PE1 (9)", "PE2 (LP)", "PF4 (4)", "PF5 (7)", "PF6 (5)", "PF7 (6)
extern const uint8_t port_bit[7];
extern const char channel_names[7][4];

#define	HARDWARE_OBJECT_PE0				0
#define INIT_HARDWARE_OBJECT_PE0		UCSR0B &= (0xff ^ (1<< RXEN0)); sbi (DDRE, 0);
#define SET1_HARDWARE_OBJECT_PE0		sbi (PORTE, 0);
#define SET0_HARDWARE_OBJECT_PE0		cbi (PORTE, 0);
#define INIT_HARDWARE_INPUT_PE0			UCSR0B &= (0xff ^ (1<< RXEN0)); cbi (DDRE, 0);
#define CHECK_HARDWARE_INPUT_PE0		(PINE & (1 << 0))

#define	HARDWARE_OBJECT_PE1				1
#define INIT_HARDWARE_OBJECT_PE1		UCSR0B &= (0xff ^ (1<< TXEN0)); sbi (DDRE, 1);
#define SET1_HARDWARE_OBJECT_PE1		sbi (PORTE, 1);
#define SET0_HARDWARE_OBJECT_PE1		cbi (PORTE, 1);
#define INIT_HARDWARE_INPUT_PE1			UCSR0B &= (0xff ^ (1<< TXEN0)); cbi (DDRE, 1);
#define CHECK_HARDWARE_INPUT_PE1		(PINE & (1 << 1))

#define	HARDWARE_OBJECT_PE2				2
#define INIT_HARDWARE_OBJECT_PE2		sbi (DDRE, 2);
#define SET1_HARDWARE_OBJECT_PE2		sbi (PORTE, 2);
#define SET0_HARDWARE_OBJECT_PE2		cbi (PORTE, 2);
#define INIT_HARDWARE_INPUT_PE2			cbi (DDRE, 2);
#define CHECK_HARDWARE_INPUT_PE2		(PINE & (1 << 2))

#define	HARDWARE_OBJECT_PF4				3
#define INIT_HARDWARE_OBJECT_PF4		sbi (DDRF, 4);
#define SET1_HARDWARE_OBJECT_PF4		sbi (PORTF, 4);
#define SET0_HARDWARE_OBJECT_PF4		cbi (PORTF, 4);
#define INIT_HARDWARE_INPUT_PF4			cbi (DDRF, 4);
#define CHECK_HARDWARE_INPUT_PF4		(PINF & (1 << 4))

#define	HARDWARE_OBJECT_PF5				4
#define INIT_HARDWARE_OBJECT_PF5		sbi (DDRF, 5);
#define SET1_HARDWARE_OBJECT_PF5		sbi (PORTF, 5);
#define SET0_HARDWARE_OBJECT_PF5		cbi (PORTF, 5);
#define INIT_HARDWARE_INPUT_PF5			cbi (DDRF, 5);
#define CHECK_HARDWARE_INPUT_PF5		(PINF & (1 << 5))

#define	HARDWARE_OBJECT_PF6				5
#define INIT_HARDWARE_OBJECT_PF6		sbi (DDRF, 6);
#define SET1_HARDWARE_OBJECT_PF6		sbi (PORTF, 6);
#define SET0_HARDWARE_OBJECT_PF6		cbi (PORTF, 6);
#define INIT_HARDWARE_INPUT_PF6			cbi (DDRF, 6);
#define CHECK_HARDWARE_INPUT_PF6		(PINF & (1 << 6))

#define	HARDWARE_OBJECT_PF7				6
#define INIT_HARDWARE_OBJECT_PF7		sbi (DDRF, 7);
#define SET1_HARDWARE_OBJECT_PF7		sbi (PORTF, 7);
#define SET0_HARDWARE_OBJECT_PF7		cbi (PORTF, 7);
#define INIT_HARDWARE_INPUT_PF7			cbi (DDRF, 7);
#define CHECK_HARDWARE_INPUT_PF7		(PINF & (1 << 7))

#define INIT_PORT_PE					DDRE &= 0xf7; PORTE &= 0xf7;
#define INIT_PORT_PF					DDRF &= 0x0f; PORTF &= 0x0f;

#endif
