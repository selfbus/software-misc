/**
 * \file TPUart.h
 *
 * \brief This module contains constant definitions for the TPUART Link Layer Driver
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef TPUART_H_
#define TPUART_H_

#include <sys/msg.h>
#include <dev/irqreg.h>
#include <sys/thread.h>
#include <sys/event.h>
#include <sys/atom.h>
#include "task.h"
#include "hardware.h" // definition of uart channel and reset

// Hardware adaption. Define EIB_UART, EIB_RESET_DDR EIB_RESET_PIN and EIB_RESET_PORT
#if (EIB_UART == 0)
#define EIB_TX_PORT	PORTE
#define EIB_TX_DDR	DDRE
#define EIB_TX_BIT	1
#define EIB_RX_PORT	PORTE
#define EIB_RX_DDR	DDRE
#define EIB_RX_BIT	0
#define EIB_RX_INT	sig_UART0_RECV
#define EIB_TX_INT	sig_UART0_DATA
#define EIB_UDR		UDR0
#define EIB_UCSRB	UCSR0B
#define EIB_UDRIE	UDRIE0
#define EIB_UBRRL	UBRR0L
#define EIB_UBRRH	UBRR0H
#define EIB_UCSRA	UCSR0A
#define EIB_UCSRB	UCSR0B
#define EIB_UCSRC	UCSR0C
#define EIB_UART_ERROR_MASK ( (1<<FE0) | (1<<DOR0) | (1<<UPE0) )
// EIB_UART == 1
#else
#define EIB_TX_PORT	PORTD
#define EIB_TX_DDR	DDRD
#define EIB_TX_BIT	3
#define EIB_RX_PORT	PORTD
#define EIB_RX_DDR	DDRD
#define EIB_RX_BIT	2
#define EIB_RX_INT	sig_UART1_RECV
#define EIB_TX_INT	sig_UART1_DATA
#define EIB_UDR		UDR1
#define EIB_UCSRB	UCSR1B
#define EIB_UDRIE	UDRIE1
#define EIB_UBRRL	UBRR1L
#define EIB_UBRRH	UBRR1H
#define EIB_UCSRA	UCSR1A
#define EIB_UCSRC	UCSR1C
#define EIB_UART_ERROR_MASK ( (1<<FE1) | (1<<DOR1) | (1<<UPE1) )
#endif
#define EIB_TIMEOUT	sig_OVERFLOW1

// re-enable UART transmit-interrupt
#define EIB_TXINT_ENABLE	EIB_UCSRB |= (1<<UDRIE);
#define EIB_TXINT_DISABLE	EIB_UCSRB &= 0xff ^ (1<<UDRIE);
#define EIB_RXINT_ENABLE	EIB_UCSRB |= (1<<RXCIE);
#define EIB_RXINT_DISABLE	EIB_UCSRB &= ~(1<<RXCIE);

// define tokens for the receive interrupt service function
#define RECV_INT	((void *)1)
#define OVL_INT		((void *)2)

//issue hardware reset request to TPUART
#define EIB_RESET_TPUART	cbi (EIB_RESET_OUT_PORT, EIB_RESET_OUT_BIT); sbi (EIB_RESET_OUT_DDR, EIB_RESET_OUT_BIT);
#define EIB_RELEASE_TPUART	sbi (EIB_RESET_OUT_DDR, EIB_RESET_OUT_BIT); sbi (EIB_RESET_OUT_PORT, EIB_RESET_OUT_BIT);
#define EIB_RESET			((EIB_RESET_PIN & (1 << EIB_RESET_IN_BIT)) > 0)

// Baud Rate divisor.
#define EIB_BAUDX	16		// Baud rate divisor.
// each 1.35ms a new message byte is expected. If the gap is larger, an ACK byte is expected
// the ACK byte arrives 2.7ms after the last message byte. After 2.5ms we are sure the last 
// message byte is really missing and the ACK byte is not yet received
#define EIB_MSG_ACK_GAP	2.25
// the ACK byte should arrive 200us after timeout for the last message byte. We add some margin.
#define EIB_ACK_LEN		1.0
// timeout for pending TX_WAIT in units of the main loop delay (currently 30ms): 90-120ms
#define EIB_MAX_ACK_TIMEOUT		4
// macro to start, stop and retrigger the timeout
#define EIB_TIMER_START(TIME)	TCNT1 = (TIME); TCCR1B = (1<<CS10); TIFR |= (1<<TOV1); TIMSK |= (1<<TOIE1);
#define EIB_TIMER_RESTART(TIME) TCNT1 = (TIME);
#define EIB_TIMER_STOP	TIMSK &= 0xff ^ (1<<TOIE1); TCCR1B = 0;

//EIB message frame for TPUART transfer
//This message format is exchanged with the Network layer
#define FRAME_LEN 23	// Ctrl + 22, we don't support long frames
typedef struct {
	int8_t 		len;				// number of valid bytes in this frame
	uint8_t		ack;				// ack state from TPUART
	uint8_t 	frame[FRAME_LEN];	// buffer for frame data in EMI format
} t_eib_frame;

enum e_eib_receiver_states
{
	RX_IDLE,		// waiting for new response from TPUART
	RX_NEXT,		// waiting for next data byte of a message
	RX_ACK,		// waiting for ACK
	RX_IACK,		// waiting for ACK, ignoring message
	RX_IGNORE		// ignore following received bytes of a message
};

enum e_eib_transmitter_states
{
	TX_IDLE,		// waiting for new message to sent
	TX_NEXT,		// sending next byte of message
	TX_CHECK,		// sending checksum of message (= last byte)
	TX_WAIT			// waiting for ACK
};

// Tokens for communication queues.
#define EIB_L_DATA_INDICATION	1
// Define amount of buffers for communication with Network Layer
#define EIB_RX_BUFFERS	16
#define EIB_TX_BUFFERS	16

//*****************************************
// codes sent to TPUART
//*****************************************
// TPUART constants. Except U_DataContinue and U_DataEnd these codes can
// be sent directly to the TPUART
#define U_NO_COMMAND			0x00	// added for driver
#define U_RESET_REQUEST			0x01
#define U_STATE_REQUEST			0x02
#define U_ACTIVATE_BUSMONITOR	0x05
#define U_ACKINFORMATION_NACK	0x15
#define U_ACKINFORMATION_ACK	0x11
#define U_ACKINFORMATION_BUSY	0x13
#define U_ACKINFORMATION_NO_ACK	0x00	// sent no ack
#define U_L_DATA_START			0x80  // || buffer (this is driver special)
#define U_L_DATA_CONTINUE		0x80  // || index [1..62]
#define	U_L_DATA_END			0x40  // || last index +1 [7..63]
#define	U_L_POLLING_STATE		0xE0  // || slotnumber [0..14]

//*****************************************
// codes sent by TPUART
//*****************************************
#define TPUART_RESET_INDICATION		0x03	// TPUART indicates reset
#define TPUART_STATE_INDICATION_MASK 0x07	// TPUART indicates its state
#define TPUART_STATE_INDICATION		0x07		
#define	TPUART_L_DATA_CONFIRM_OK	0x8B	// TPUART positive confirm
#define TPUART_L_DATA_CONFIRM_NG	0x0B	// TPUART negative confirm
#define TPUART_L_DATA_CONFIRM		0x0B	// TPUART negative confirm
#define TPUART_L_DATA_NO_CONFIRM	0x00	// TPUART did not sent confirm
#define TPUART_L_DATA_CONFIRM_MASK	0x7F	// TPUART negative confirm
#define	TPUART_L_DATA_INDICATION	0xC0	// TPUART data indication (artificial value)

// following codes are handled by the driver level
#define EIB_INDICATION_MASK		0x13	// mask for the significant bits
#define EIB_INDICATION			0x10	// significant bits for xxxData.indication

// external function for address check must be defined in project. It is called
// by the receive interrupt function to determine, if an ACK should be issued to
// the EIB. Return value 0: no ACK, 1: sent ACK
// In case of transmission errors, the TPUART will generate NACK independent of the
// ACK request of this driver.
extern unsigned char eib_check_group_address (uint16_t);

/**
* @brief virtual channels of EIB interface
*
* 0: device channel (logic, visu)
* 1: EIBNet/IP channel
*/
#define EIB_VIRTUAL_DEVICES	2

// interface to other modules
// EIB messages are always copied. The caller of the functions has to provide pointers to
// his message buffer space. The send- and receive queues of this driver are isolated

// control interface
enum eib_command_codes { EIB_INIT_CMD, EIB_RESET_CMD, EIB_BUSMON_CMD, EIB_STATE_CMD };
char eib_control(enum eib_command_codes);
enum e_eib_tpuart_states
{
	EIB_POWER_DOWN,	// the EIB power is missing, reset is low
	EIB_NORMAL,		// the TPUART is ready in normal mode
	EIB_BUSMON		// the TPUART operates in bus monitor mode
};
enum e_eib_tpuart_states eib_get_status (void);

//copies message into transmission buffer. Returns 1, if ok; returns 0, if buffer was full
//virtual device channel has to be submitted as argument
char eib_L_DATA_request(t_eib_frame*, uint8_t);

//retrieves message from reception buffer. Returns 1, if ok; returns 0, if buffer was empty
char eib_L_DATA_indication_poll (t_eib_frame*);
//retrieves message from reception buffer. Waits until message is available
void eib_L_DATA_indication_wait (t_eib_frame*);

//set device address of virtual channel
uint8_t eib_set_device_address (uint8_t, uint16_t);
//get device address of virtual channel
uint16_t eib_get_device_address (uint8_t);

//check, if the TX buffer is less than half full
uint8_t eib_check_tx_space (void);

// check, if TX is in deadlock state and restart, if needed.
void eib_check_tx_deadlock(void);


#endif /* TPUART_H_ */
