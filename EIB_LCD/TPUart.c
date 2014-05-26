/**
 * \file TPUart.c
 *
 * \brief Link Layer Driver for TPUART
 * This module is part of the EIB-LCD Controller Firmware
 *
 * The interface to this driver is limited to functions to forward new messages
 * into the transmission buffer, get messages from the reception buffer, control
 * the driver function and get the driver status. Direct access to the variables 
 * of this driver is not needed and should be avoided.
 *
 * The function eib_L_DATA_request forwards new transmit messages into the transmit
 * message queue. Contents of the transmit message are copied, therefore the caller
 * can immediately reuse the submitted message buffer. The return value informs about
 * sucess, if the message buffer was not full.
 *
 * The functions eib_L_DATA_indication_wait and eib_L_DATA_indication_poll retrieve
 * received messages from the reception buffer. The ack field of the message contains
 * the acknowledge information of messages sent by TPUART. Messages received from other
 * nodes via the EIB contain ack information in BUSMON mode of TPUART only.
 * This driver doesn't support long data frames and polling.
 * Maximum frame length is Ctrl + 22.
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "TPUart.h"
#include <string.h>
#include <stdio.h>

//global variable definitions
enum e_eib_tpuart_states		eib_state;		//state of the TPUART system
enum e_eib_receiver_states		eib_recv_state;	//state of the TPUART receiver state machine
uint8_t							eib_rx_checksum;//checksum of RX message

//message buffers for Network Layer communication
t_eib_frame		eib_rx_buffer[EIB_RX_BUFFERS];
t_eib_frame		eib_tx_buffer[EIB_TX_BUFFERS];
int				eib_rx_out, eib_rx_in;	// pointers for receive message buffer queue
int				eib_tx_out, eib_tx_in;	// pointers for transmit message buffer queue

enum e_eib_transmitter_states		eib_trans_state;	//state of the TPUART transmitter state machine
//byte counter for tx function
int				eib_tx_buf_i;			// index of next message byte of tx message
// following variables manage the message data transmission of ctrl + data
uint8_t			eib_tx_msg_byte_value;	// data byte to be sent next
uint8_t			eib_tx_msg_byte_flag;	// 0= no data, 1= eib_tx_msg_byte must be sent next
uint8_t			eib_tx_checksum;		// checksum byte for transmission
//last status information from TPUART
uint8_t			eib_tpuart_state;		// contents of TPUART State.inidcation/Response
char			eib_new_tpuart_state;	// flags new status response from TPUART
char			eib_tpuart_reset;		// reset indication received from TPUART
//calculate constants for timeout
u_short eib_msg_gap_time; // time between two bytes of a message
u_short eib_ack_len_time; // time between last message byte time out and the ACK byte	
//sent ACK for running frame to TPUART
u_char eib_ack_information;
//timeout for waiting on ACK
u_char eib_ack_timeout;
// command to TPUART
char eib_tpuart_cmd;
// create queues for message handling
static HANDLE eib_tx_event;
static HANDLE eib_rx_event;

// stores physical addresses of virtual devices
uint16_t	device_address[EIB_VIRTUAL_DEVICES];

// stores next byte in the active buffer.
char eib_store_byte (unsigned char value)
{
	if (eib_rx_buffer[eib_rx_in].len < FRAME_LEN) {
		eib_rx_buffer[eib_rx_in].frame[eib_rx_buffer[eib_rx_in].len++] = value;
		eib_rx_checksum ^= value;
		return 1;
	}
	// error: frame buffer overrun
	return 0;
}


/**
* @brief checks, if device is addressed
*
* The physical addresses of all virtual devices are checked in this function.
* Group address check is located in the upper driver layers
*/
inline uint8_t eib_check_address (void) {

int i;
uint16_t saddr, daddr;

	if (eib_state != EIB_NORMAL) return 0;

	saddr = eib_rx_buffer[eib_rx_in].frame [1] | (eib_rx_buffer[eib_rx_in].frame [2] << 8);
	daddr = eib_rx_buffer[eib_rx_in].frame [3] | (eib_rx_buffer[eib_rx_in].frame [4] << 8);

	// no ACK for my own messages
	for (i=0; i<EIB_VIRTUAL_DEVICES; i++)
		if (saddr == device_address[i])
			return 0;

	if ( eib_rx_buffer[eib_rx_in].frame [5] & 0x80) {
		//group address
		return eib_check_group_address (daddr);
	} 
	else {
		// device address
		for (i=0; i<EIB_VIRTUAL_DEVICES; i++)
			if ( daddr == device_address[i])
				return 1;
		}
	return 0;
}


//UART receive and TIMER timeout interrupt
//Caution: timer interrupt has priority over the UART receiver interrupt!
static void eib_rx_interrupt (void *arg)
{
uint8_t rx_byte = 0;	// new data byte
uint8_t rx_flags = 0; 	// error flags of new data byte

	if (arg == RECV_INT) {
		rx_flags = EIB_UCSRA & EIB_UART_ERROR_MASK;		// read error flags
		rx_byte = EIB_UDR;			// read data and clear Rx interrupt
	}
	else {
		// stop timer
		EIB_TIMER_STOP
	}

	// we receive someting, restart TX deadlock checker
	eib_ack_timeout = 0;

	// check receiver state machine
	switch (eib_recv_state) {
		case RX_NEXT:
			// we are receiving a message
			if (arg == RECV_INT) {
				// trace telegramm length
				EIB_TIMER_RESTART (eib_msg_gap_time)
				// store new byte to buffer
				if (rx_flags || (!eib_store_byte (rx_byte)))
					eib_recv_state = RX_IGNORE;
				else if (eib_rx_buffer[eib_rx_in].len == 6) {
					if (eib_check_address () ) {
						// send ACK response to TPUART
						eib_ack_information = U_ACKINFORMATION_ACK;
						EIB_TXINT_ENABLE
					}
				}
			}
			else	// event has been time out: end of message. Wait for ACK now
			{
				// set buffer state to wait for ACK
				eib_recv_state = RX_ACK;
				// start new timeout for message data bytes
				EIB_TIMER_START (eib_ack_len_time)
			}
		break;
		case RX_IGNORE:
			if (arg == RECV_INT) {
				// trace telegramm length
				EIB_TIMER_RESTART (eib_msg_gap_time)
			}
			else	// event has been time out: end of message. Wait for ACK now
			{
				eib_recv_state = RX_IACK;
				// start new timeout for message data bytes
				EIB_TIMER_START (eib_ack_len_time)
			}
		break;

		case RX_ACK:
		case RX_IACK:

			// we are waiting for an ACK
			// this ACK is only supplied after the TPUART transmitted a message to the bus.
			// In case of a receive message from the bus, no ACK is sent by TPUART. This
			// condition is detected by a timeout.

			// was it a time out?
			if (arg == RECV_INT) {
				// store ACK to RX buffer, if no UART error flags
				if (!rx_flags)
					eib_rx_buffer[eib_rx_in].ack = rx_byte;
				// received confirmation for last TX message
				eib_trans_state = TX_IDLE;
				// trigger transmitter to sent next TX message to TPUART
				NutEventPostFromIrq (&eib_tx_event);
			}
			// mark this buffer as completed, if the checksum is ok
			if ((eib_rx_checksum == 0xff) && (eib_recv_state == RX_ACK)) {
				if (++eib_rx_in >= EIB_RX_BUFFERS)
					eib_rx_in = 0;
				NutEventPostFromIrq (&eib_rx_event);
			}

			// end of this frame
			EIB_TIMER_STOP
			// set receiver state to idle
			eib_recv_state = RX_IDLE;
		break;
		case RX_IDLE:
			// we are not receiving.
			if ((arg == OVL_INT) || (rx_flags)) {
				// timer has already been stopped at function entry
				break;
			}
			// The reset indication will be received during normal operation only. 
			// On power-up, UART init sequence makes it usually impossible to receive this byte.
			if (rx_byte == TPUART_RESET_INDICATION) {
				eib_tpuart_reset = 1;
				eib_state = EIB_NORMAL;
				//restart transmitter
				eib_tx_msg_byte_flag = 0;
				eib_trans_state = TX_IDLE;
				NutEventPostFromIrq(&eib_tx_event);
				break;
			}
			if ((rx_byte & TPUART_STATE_INDICATION_MASK) == TPUART_STATE_INDICATION) {
				eib_tpuart_state = rx_byte;
				eib_new_tpuart_state = 1;
				break;
			}
			//Check type of byte
			if ( (rx_byte & EIB_INDICATION_MASK) == EIB_INDICATION ) {

				// new data frame starts
				// start timeout
				EIB_TIMER_START (eib_msg_gap_time)
				// check for new receive buffer
				int i = eib_rx_in +1;
				if (i >= EIB_RX_BUFFERS)
					i = 0;
				if (i == eib_rx_out) {
					// overflow, no free buffer available
					eib_recv_state = RX_IGNORE;
					// send BUSY response to TPUART
					eib_ack_information = U_ACKINFORMATION_BUSY;
					EIB_TXINT_ENABLE
					break;
				}
				// receive message
				eib_recv_state = RX_NEXT;
				eib_rx_buffer[eib_rx_in].ack = TPUART_L_DATA_NO_CONFIRM;
				eib_rx_buffer[eib_rx_in].len = 0;
				eib_ack_information = U_ACKINFORMATION_NO_ACK;
				// start checksum calculation
				eib_rx_checksum = 0;
				// store new byte to buffer
				eib_store_byte (rx_byte);
			}
		break;
		default:
			// for security
			eib_recv_state = RX_IDLE;
			EIB_TIMER_STOP
		}

}

// transmit shift register empty interrupt from UART
// Now we can sent two bytes, since Tx shift register and Tx buffer
// are both empty.
static void eib_tx_interrupt (void *arg)
{
	//check, if data byte must be sent after ctrl byte has been sent
	if (eib_tx_msg_byte_flag) {
		EIB_UDR = eib_tx_msg_byte_value;
		eib_tx_msg_byte_flag = 0;
	}
	// check, if command should be sent to TPUART
	else if (eib_tpuart_cmd) {
		EIB_UDR = eib_tpuart_cmd;
		eib_tpuart_cmd = U_NO_COMMAND;
	}
	// check, if immediate ACK should be sent to TPUART
	else if (eib_ack_information) {
		EIB_UDR = eib_ack_information;
		eib_ack_information = U_ACKINFORMATION_NO_ACK;
	}
	else switch (eib_trans_state) {
		case TX_NEXT:
			//sent next ctrl and data byte to TPUART
			eib_tx_msg_byte_value = eib_tx_buffer[eib_tx_out].frame[eib_tx_buf_i];
			eib_tx_msg_byte_flag = 1;
			//update checksum
			eib_tx_checksum ^= eib_tx_msg_byte_value;
			EIB_UDR = U_L_DATA_CONTINUE | eib_tx_buf_i++;
			//last byte is the checksum
			if (eib_tx_buffer[eib_tx_out].len == eib_tx_buf_i)
				eib_trans_state = TX_CHECK;
		break;
		case TX_CHECK:
			// send checksum
			eib_tx_msg_byte_value = ~eib_tx_checksum;
			eib_tx_msg_byte_flag = 1;
			EIB_UDR = U_L_DATA_END | eib_tx_buf_i;
			// next state is waiting for the ACK from TPUART
			eib_trans_state = TX_WAIT;
			eib_ack_timeout = 0;
			// next send buffer
			if (++eib_tx_out >= EIB_TX_BUFFERS)
				eib_tx_out = 0;
		break;
		default: 
			EIB_TXINT_DISABLE
	}
}


THREAD(eib_process_tx_queue, arg)
{
#define MAX_TX_WAIT 3000	// timeout 3s

	NutThreadSetPriority(NUT_THREAD_PRIORITY_EIB_SERVE_TX);
    /*
     * Now loop endless for new EIB TX messages
     */


    for (;;) {

		NutEventWait (&eib_tx_event, NUT_WAIT_INFINITE);
		// are we online?
		if ((eib_state == EIB_NORMAL) && ( eib_trans_state == TX_IDLE )
			  && (eib_tx_out != eib_tx_in) ) {
			// we have a new message and no pending transmission

			// start sending
			eib_tx_buf_i = 0; // index of next message byte of tx message
			eib_tx_checksum = 0;
			eib_tx_msg_byte_flag = 0; // for security
			eib_trans_state = TX_NEXT;
			EIB_TXINT_ENABLE
		}
	}
}


// checks the state of the TPUART system and updates internal
// flags accordingly.
// Note: this function must be called frequently to keep the
//       driver status up to date.
void eib_check_state (void)
{
	if EIB_RESET {
		// the TPUART is either in reset state or powered down
		eib_state = EIB_POWER_DOWN;
		// disable uart interrupts and port pins
		EIB_TXINT_DISABLE
		EIB_RXINT_DISABLE
		// stop UART
		EIB_UCSRB = 0;
		EIB_TIMER_STOP

		eib_recv_state = RX_IDLE;
		eib_trans_state = TX_WAIT;
		eib_new_tpuart_state = 0;
		eib_tx_msg_byte_flag = 0;
		eib_ack_timeout = 0;
		eib_tx_msg_byte_flag = 0;
	}
	else {
	// the TPUART is powered up
		if (eib_state == EIB_POWER_DOWN) {
			eib_ack_information = U_ACKINFORMATION_NO_ACK;
			// enable UART
			EIB_UCSRA = (EIB_BAUDX == 8)?(1<<U2X):0;
			EIB_UCSRB = ((1<<TXEN) | (1<<RXEN));
			EIB_UCSRC = ((1<<UPM1) | (1<<UCSZ1) | (1<<UCSZ0));

			// One USART, (C register shared), 8e1
			u_long	rate = 19200;
		    u_short sv;
		    if (bit_is_clear(EIB_UCSRC, UMSEL)) {
		        if (bit_is_set(EIB_UCSRA, U2X)) {
		            rate <<= 2;
		        } else {
		            rate <<= 3;
		        }
		    }
		    sv = (u_short) ((NutGetCpuClock() / rate + 1UL) / 2UL) - 1;

			EIB_UBRRL = sv;
			EIB_UBRRH = (uint8_t)(sv>>8);

			// enable uart rx interrupt and port pins
			eib_state = EIB_NORMAL;
			EIB_RXINT_ENABLE
			// add system log entry
		}
	}
}


/*************************************************************
	Interface functions to other modules
*************************************************************/



/**
* @brief put L_DATA to transmission buffer
*
* Puts message into the send queue.<br>
* Allocates transmission buffer by eib_allocate_buffer<br>
* Check sum is calculated by the transmit function. Do not
* include checksum in the message forwarded to eib_L_DATA_request.<br>
* Clears the ACK field of the message<br>
*/
char eib_L_DATA_request (t_eib_frame *msg, uint8_t channel) {

int i;

	if (channel >= EIB_VIRTUAL_DEVICES)
		return 0;

	i = eib_tx_in+1;
	if (i >= EIB_TX_BUFFERS)
		i = 0;
	NutEnterCritical();
	if (i == eib_tx_out) {
		//buffer overflow, ignore message
		NutExitCritical();
		return 0;
	}
	NutExitCritical();

	// copy message into transmission buffer
	if (msg->len > FRAME_LEN)
		return -1;
	memcpy (&(eib_tx_buffer[eib_tx_in].frame[0]), &(msg->frame[0]), msg->len);	
	// set my device address
	eib_tx_buffer[eib_tx_in].frame[1] = device_address[channel] & 0xff;
	eib_tx_buffer[eib_tx_in].frame[2] = (device_address[channel] >> 8) & 0xff;
	eib_tx_buffer[eib_tx_in].len = msg->len;

	// avoid interruption by interrupt services
	NutEnterCritical();
	eib_tx_in = i;
	NutExitCritical();
	NutEventPost (&eib_tx_event);
	return 1;
}

/**
* @brief check for new L_DATA
*
* Get message from receive queue. Returns 1, if new message was available.
* Messages are L_DATA_indication or L_DATA_confirm.
*/
char eib_L_DATA_indication_poll (t_eib_frame* msg)
{
	if (eib_rx_in == eib_rx_out)
		return 0;

	memcpy (msg, &(eib_rx_buffer[eib_rx_out]), sizeof(t_eib_frame)-FRAME_LEN+eib_rx_buffer[eib_rx_out].len);
	NutEnterCritical();
	if (++eib_rx_out >= EIB_RX_BUFFERS)
		eib_rx_out = 0;
	NutExitCritical();
	return 1;
}

/**
* @brief wait for new L_DATA
*
* Get message from receive queue. Waits until new message is available.
* Messages are L_DATA_indication or L_DATA_confirm
*/
void eib_L_DATA_indication_wait (t_eib_frame* msg)
{
	while (!eib_L_DATA_indication_poll (msg))
		NutEventWait (&eib_rx_event, NUT_WAIT_INFINITE);
}


/**
* @brief get status of EIB driver
*
* This function should be called frequently to update the driver state.
*
*/
enum e_eib_tpuart_states eib_get_status (void) {
	
	eib_check_state ();
	return eib_state;
}

/**
* @brief check status of EIB driver TX
*
* This function should be called frequently to detect and solve a TX_WAIT driver state deadlock.
*
*/
void eib_check_tx_deadlock(void) {
	if ((eib_trans_state == TX_WAIT) && (++eib_ack_timeout > EIB_MAX_ACK_TIMEOUT)) {
		//restart transmitter
		eib_trans_state = TX_IDLE;
		NutEventPost(&eib_tx_event);
	}
}


/**
* @brief sends commands to EIB driver
* 
* commands defined by enum:
*  -reset TPUART
*  -switch to bus monitor mode
*  -reset TPUART
*/
char eib_control (enum eib_command_codes cmd) {
	
	switch (cmd) {
		//Init the TPUART EIB driver:
		// -initialize message buffers for communication with Network Layer
		// -initialize the interrupt handler for UART Tx and Rx
		// -initialize the IO port functions for RESET\, TxD and RxD
		case EIB_INIT_CMD:
			EIB_RESET_TPUART
			eib_tpuart_reset = 0;
			eib_tpuart_state = 0;
			eib_new_tpuart_state = 0;
			eib_tx_msg_byte_flag = 0;
			eib_state = EIB_POWER_DOWN;

			// calculate time out between EIB messages
		    eib_msg_gap_time = (u_short) (0xFFFF - (((EIB_MSG_ACK_GAP)*NutGetCpuClock())/1000) );
		    eib_ack_len_time = (u_short) (0xFFFF - (((EIB_ACK_LEN)*NutGetCpuClock())/1000) );
		    // register all interrupt vectors. Should never fail.
		    NutRegisterIrqHandler(&EIB_RX_INT, eib_rx_interrupt, RECV_INT );
		    NutRegisterIrqHandler(&EIB_TIMEOUT, eib_rx_interrupt, OVL_INT );
		    NutRegisterIrqHandler(&EIB_TX_INT, eib_tx_interrupt, NULL );

			// create send process
			NutThreadCreate("EIB_TX", eib_process_tx_queue, 0, NUT_THREAD_EIB_TX_STACK);
			eib_tpuart_cmd = U_NO_COMMAND;
			eib_recv_state = RX_IDLE;
			EIB_RELEASE_TPUART
		break;

		case EIB_RESET_CMD:
			eib_tpuart_cmd = U_RESET_REQUEST;
			// restart transmitter
			EIB_TXINT_ENABLE
		break;

		case EIB_BUSMON_CMD:
			eib_tpuart_cmd = U_ACTIVATE_BUSMONITOR;
			EIB_TXINT_ENABLE
			eib_state = EIB_BUSMON;
		break;

		case EIB_STATE_CMD:
			eib_tpuart_cmd = U_STATE_REQUEST;
			EIB_TXINT_ENABLE
		break;
	}
	return 0;
}

/**
* @brief set virtual device address
*
* This function sets the physical address of a virtual channel.
* In case of a valid channel it returns 1, otherwise 0.
*/
uint8_t eib_set_device_address (uint8_t channel, uint16_t address) {
	if (channel >= EIB_VIRTUAL_DEVICES)
		return 0;

	device_address[channel] = address;
	return 1;
}

/**
* @brief get virtual device address
*
* This function returns the physical address of a virtual channel.
* In case of an invaid channel it returns 0x0000.
*/
uint16_t eib_get_device_address (uint8_t channel) {
	if (channel >= EIB_VIRTUAL_DEVICES)
		return 0;

	return device_address[channel];
}


/**
* @brief check, if the TX buffer is less than half full
*
* This function returns 1, if more than half of the TX buffer is free.
* It returns 0, if more than half of the TX buffer is already occupied.
*/
uint8_t eib_check_tx_space (void) {

uint8_t tx_used;

	NutEnterCritical();
	if (eib_tx_in >= eib_tx_out)
		tx_used = eib_tx_in - eib_tx_out;
	else tx_used = EIB_TX_BUFFERS - (eib_tx_out - eib_tx_in);
	NutExitCritical();

	if (tx_used < (EIB_TX_BUFFERS / 2))
		return 1;

	return 0;
}
