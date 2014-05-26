/** \file EIBLayers.c
 *  \brief Functions for EIB protocol layers above Link layer
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- 
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "System.h"

char eib_TL_DATA_request_ACK(uint8_t*, uint8_t);

/***************************************/
/* Layer 4 (Application Layer) support */
/***************************************/

//data buffer for replies
uint8_t		al_data[EIB_AL_DATA_LEN];


uint8_t al_get_mem (uint16_t maddr) {

	if (maddr == MADDR_STATUS_BYTE)
		return 0x00;

	if (maddr == MADDR_BCU_DATA_BYTE_0) 
		return 0xFF;

	if (maddr == MADDR_BCU_DATA_BYTE_1) 
		return 0xFF;

	if (maddr == MADDR_BCU_DATA_BYTE_2) 
		return 0xFF;

	if (maddr == MADDR_MANUF_BYTE) 
		return 0xFF;

	// no emulated memory location
	return 0xff;
}

void al_data_indication (t_eib_frame* msg) {

uint16_t	apci;
uint16_t	m_start;
uint8_t		m_len;
uint8_t		i;
uint8_t		adc_channel;
uint8_t		adc_repeat;
uint16_t	adc_result;

	// Process APCI, if data is in sequence
	apci = ((msg->frame[TPDU_POSITION] & 0x03) << 8) | msg->frame[APCI_POSITION];
//printf_P(PSTR("apci(%x) "), apci);
	switch (apci) {
		case A_READ_MASK_VERSION_REQ_PDU:
//printf_P(PSTR("MASK "));
 			// sent Mask
			al_data[0] = (0x03 & (A_READ_MASK_VERSION_RES_PDU >> 8));
			al_data[1] = 0xff & A_READ_MASK_VERSION_RES_PDU;
			al_data[2] = DEVICE_MASK_TYPE;
			al_data[3] = DEVICE_MASK_VERSION;
			eib_TL_DATA_request_ACK(&al_data[0], 4);
		break;
		default:
			// may fit to memory read
			if ((apci & A_MEM_MASK) == A_READ_MEM_REQ_PDU) {
				m_len = apci & A_READ_MEM_LEN_MASK;
				m_start = (msg->frame[APCI_POSITION +1] << 8) | msg->frame[APCI_POSITION +2];
//printf_P(PSTR("MEM [%4x] %i "), m_start, m_len);
	 			// sent memory contents
				al_data[0] = (0x03 & (A_READ_MEM_RES_PDU >> 8));
				al_data[1] = (0xff & A_READ_MEM_RES_PDU) | m_len;
				al_data[2] = msg->frame[APCI_POSITION +1];
				al_data[3] = msg->frame[APCI_POSITION +2];
				for (i=0; i<m_len; i++)
					al_data[i+4] = al_get_mem (i+m_start);
				eib_TL_DATA_request_ACK(&al_data[0], 4+m_len);
				break;
			}
			// may fit to ADC read
			if ((apci & A_ADC_MASK) == A_READ_ADC_REQ_PDU) {
				adc_channel = apci & A_ADC_CHANNEL_MASK;
				adc_repeat = msg->frame[APCI_POSITION +1];
				adc_result = 0;
				for (i=0; i<adc_repeat; i++)
					adc_result += 0x80; //FIXME: real ADC conversion result here!
	 			// sent ADC result
				al_data[0] = (0x03 & (A_READ_ADC_RES_PDU >> 8));
				al_data[1] = (0xff & A_READ_ADC_RES_PDU) | adc_channel;
				al_data[2] = adc_repeat;
				al_data[3] = adc_result >> 8;
				al_data[4] = adc_result & 0xff;
				eib_TL_DATA_request_ACK(&al_data[0], 5);
				break;
			}

//printf_P(PSTR("?APCI %4X? "), apci);
	}
}


/*************************************/
/* Layer 4 (Transport Layer) support */
/*************************************/
/* adress of connected device */
uint8_t		connection_address_L;
uint8_t		connection_address_H;
/* 4 bit value for sequence number of T_DATA_XXX_PDU */
uint8_t		SeqNoSend;
/* 4 bit value for sequence number of T_DATA_XXX_PDU */
uint8_t		SeqNoRcv;
/* TL state machine status */
EIB_TL_STATES	eib_tl_state;
/* Timeout counters for acknowledge and connection */
uint8_t		acknowledgement_timeout_timer;
uint8_t		connection_timeout_timer;
/* buffer for responses */
t_eib_frame tl_send_msg;
uint8_t		tl_retries;
/* L4 state machine states */
/* time interval of 6 s;
 -starts with transition CLOSED�CONNECTING;
 -stops with transition DISCONNECTING�CLOSED;
 -restarts if N_Data.req is applied in the state machine or N_Data.ind received with source_address=connection_address*/
/* time interval of 3 s;
 -starts with transition
 -OPEN_IDLE�OPEN_WAIT_FOR_T_DATA_ACK;
 -stops if N_DATA.ind & source_address=connection_address & nsdu=T_DATA_ACK_PDU & SeqNo_of_PDU=SeqNoRcv received 
	or with transition into CLOSED */
/* used to count the number of T_DATA_REQ repetitions */
uint8_t		rep_count;

/**
* @brief Sends TL message. Returns 1, if ok; returns 0, if buffer was full
* ah, al: destination address
* *data: pointer to transmit data
* len: len of transmit data: 0=0..6 bit, 1=1byte, 2=2byte, etc
*/
char eib_TL_DATA_request(uint8_t ah, uint8_t al, uint8_t *data, uint8_t len) {

	if (!len)
		return 0;

	((t_eib_message*)&(tl_send_msg.frame))->ctrl = 0xB0;
	tl_send_msg.frame [EIB_DEST_ADDRESS_HIGH] = ah;
	tl_send_msg.frame [EIB_DEST_ADDRESS_LOW] = al;
	((t_eib_message*)&(tl_send_msg.frame))->NPCI = (len-1) & 0x0f;
	*data |= TPDU_NUMBERED_DATA | (SeqNoSend << TPDU_SEQUENCE_OFFSET);

	memcpy( &tl_send_msg.frame[TPDU_POSITION], data, len);

	//set message length
	tl_send_msg.len = len + 6;

	// retrigger connection timeout
	connection_timeout_timer = (EIB_TL_CONNECTION_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL);

	// insert the routing counter
	return eib_N_DATA_request (&tl_send_msg);
}

/**
* @brief Sends TL message. Returns 1, if ok; returns 0, if buffer was full or no ACK has been received
* ah, al: destination address
* *data: pointer to transmit data
* len: len of transmit data: 0=0..6 bit, 1=1byte, 2=2byte, etc
*/
char eib_TL_DATA_request_ACK(uint8_t *data, uint8_t len) {

	//this is the first try
	tl_retries = 0;
	//set timeout
	acknowledgement_timeout_timer = EIB_TL_ACKNOWLEDGE_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL;
	//retrigger connection timeout
	connection_timeout_timer = (EIB_TL_CONNECTION_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL);
	// set state machine to ACK wait state
	eib_tl_state = OPEN_WAIT_FOR_T_DATA_ACK;
	// try to sent message
	return eib_TL_DATA_request(connection_address_H, connection_address_L, data, len);
}

/**
* @brief sets communction state machine
*/
void eib_TL_close_communication (void) {
	
	eib_tl_state = CLOSED;

//printf_P(PSTR("close\n"));

}

/**
* @brief sends disconnect request to addr_h/addr_l. Returns 1, if ok; returns 0, if buffer was full
*/
char eib_tl_disconnect (uint8_t addr_h, uint8_t addr_l) {

t_eib_frame msg;

	((t_eib_message*)&(msg.frame))->ctrl = 0xB0;
	msg.frame [EIB_DEST_ADDRESS_HIGH] = addr_h;
	msg.frame [EIB_DEST_ADDRESS_LOW] = addr_l;
	((t_eib_message*)&(msg.frame))->NPCI = 0;
	msg.frame [TPDU_POSITION] = TPDU_CLOSE_CONNECTION;
	//set message length
	msg.len = TL_CTRL_MSG_LEN;
	// insert the routing counter
	return eib_N_DATA_request (&msg);
}

char eib_tl_send_ack (uint8_t sequence) {

t_eib_frame msg;

	((t_eib_message*)&(msg.frame))->ctrl = 0xB0;
	msg.frame [EIB_DEST_ADDRESS_HIGH] = connection_address_H;
	msg.frame [EIB_DEST_ADDRESS_LOW] = connection_address_L;
	((t_eib_message*)&(msg.frame))->NPCI = 0;
	if ((sequence == SeqNoRcv) || (((sequence-1)& TPDU_SEQUENCE_MASK) == SeqNoRcv))
		msg.frame [TPDU_POSITION] = TPDU_ACK_DATA | ((sequence & TPDU_SEQUENCE_MASK) << TPDU_SEQUENCE_OFFSET);
	else 
		msg.frame [TPDU_POSITION] = TPDU_NACK_DATA | ((sequence & TPDU_SEQUENCE_MASK) << TPDU_SEQUENCE_OFFSET);
	//set message length
	msg.len = TL_CTRL_MSG_LEN;
	// insert the routing counter
	return eib_N_DATA_request (&msg);
}


/**
 * @brief EIB Transport Layer receive function
 *
 * This function waits for EIB TL messages, generates the respective response 
 * depending on the incomming message and the TL state machine and controls 
 * the TL state machine states.
 *
 */
void eib_TL_data_indication (t_eib_frame* msg) {

uint8_t	tpdu;
uint8_t rx_sequence;

	// sort TPDU
	tpdu = msg->frame[TPDU_POSITION] & TPDU_MASK;

	// Data packet
	if (tpdu == TPDU_UDT)
		return;
//FIXME: avoid problems during ETS scan
return;	

	// control data (open/close)
	if (tpdu == TPDU_UCD) {

		// is it a connection request?
		if ((msg->len == TL_CTRL_MSG_LEN+1) && (msg->frame[TPDU_POSITION] == TPDU_OPEN_CONNECTION)) {
			// process message according to the state machine states
			if (eib_tl_state == CLOSED) {

				connection_address_H = msg->frame [EIB_SRC_ADDRESS_HIGH];
				connection_address_L = msg->frame [EIB_SRC_ADDRESS_LOW];
//printf_P(PSTR("open %x %x "), connection_address_H, connection_address_L);
				connection_timeout_timer = (EIB_TL_CONNECTION_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL);
				eib_tl_state = OPEN_IDLE;
				SeqNoSend = 0;
				SeqNoRcv = 0;
				return;
			}
			// error, we do not accept additional connections
			eib_tl_disconnect (msg->frame [EIB_SRC_ADDRESS_HIGH], msg->frame [EIB_SRC_ADDRESS_LOW]);
			return;
		}

		// is it a disconnect request?
		if ((msg->len == TL_CTRL_MSG_LEN+1) && (msg->frame[TPDU_POSITION] == TPDU_CLOSE_CONNECTION)) {
			// close communication
			eib_TL_close_communication ();
			return;
		}

	}

	// ignore all other messages, if connection is not open or not sent from our communication partner
	if ((connection_address_H != msg->frame [EIB_SRC_ADDRESS_HIGH]) || 
		(connection_address_L != msg->frame [EIB_SRC_ADDRESS_LOW])  ||
		(eib_tl_state == CLOSED) )
		return;

	//retrigger connection timeout
	connection_timeout_timer = (EIB_TL_CONNECTION_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL);

	// Numbered data packet
	if (tpdu == TPDU_NDT) {

		rx_sequence = (msg->frame[TPDU_POSITION] >> TPDU_SEQUENCE_OFFSET) & TPDU_SEQUENCE_MASK;
//printf_P(PSTR("seq(%x-%x) "), rx_sequence, SeqNoRcv);
		//send ACK
		eib_tl_send_ack (rx_sequence);

		// is it the expected sequence?
		if (rx_sequence == SeqNoRcv) {
			// next sequence
			SeqNoRcv = (SeqNoRcv+1) & TPDU_SEQUENCE_MASK;
			
			al_data_indication (msg);
		}
		return;
	}

	// numbered control data (ACK/NACK)
	if ((tpdu == TPDU_NCD) && (msg->len == TL_CTRL_MSG_LEN+1)) {

		// did we receive an NACK?
		if ((msg->frame[TPDU_POSITION] & TPDU_FULL_MASK) == TPDU_NACK_DATA) {
			// close communication
			eib_TL_close_communication ();
			return;
		}

		// did we receive an ACK?
		if ((msg->frame[TPDU_POSITION] & TPDU_FULL_MASK) == TPDU_ACK_DATA) {
			// check sequence
			if (((msg->frame[TPDU_POSITION] >> TPDU_SEQUENCE_OFFSET) & TPDU_SEQUENCE_MASK) == SeqNoSend) {
				// calculate next sequence number
//printf_P(PSTR("ACK(%x) "), SeqNoSend);
				SeqNoSend = (SeqNoSend+1) & TPDU_SEQUENCE_MASK;
				eib_tl_state = OPEN_IDLE;
				acknowledgement_timeout_timer = 0;
				connection_timeout_timer = (EIB_TL_CONNECTION_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL);
			}
			else {
//printf_P(PSTR("unexp. Sequence "));
			}
		}
		return;
	}
}


/**
 * @brief EIB Transport Layer timeout thread
 *
 * The endless loop in this thread supports timeout functions for the state machine 
 * supporting connection oriented communication. Timeout is used to detect communication 
 * timeouts and breakdown.
 *
 */
THREAD(EIB_TL_Service, arg)
{

    NutThreadSetPriority(NUT_THREAD_PRIORITY_EIB_TL_SERVICE);
    /*
     * Now loop endless for new EIB messages
     */
	for (;;) {

		// wait for next timeout event
		NutSleep(EIB_TL_TIMEOUT_INTERVAL);

		if (eib_tl_state != CLOSED) {
		
			// check timeout for acknowledgement
			if (acknowledgement_timeout_timer) {
				acknowledgement_timeout_timer--;
				if (!acknowledgement_timeout_timer) {
					// check repetition counter
					if (tl_retries < TL_MAX_MSG_TRIES) {
						// next retry
						tl_retries++;
						//set timeout
						acknowledgement_timeout_timer = EIB_TL_ACKNOWLEDGE_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL;
						//retrigger connection timeout
						connection_timeout_timer = (EIB_TL_CONNECTION_TIMEOUT / EIB_TL_TIMEOUT_INTERVAL);
						// resend the message
						eib_N_DATA_request (&tl_send_msg);
					 }
					 else {
					 	// terminate connection
						eib_tl_disconnect (connection_address_H, connection_address_L);
						eib_TL_close_communication();
					 }
				}
			}

			// check timeout for connection
			if (connection_timeout_timer) {
				connection_timeout_timer--;
				if (!connection_timeout_timer) {
				 	// terminate connection
					eib_tl_disconnect (connection_address_H, connection_address_L);
					eib_TL_close_communication();
				}
			}
		}
	}
}




/***********************************/
/* Layer 3 (Network Layer) support */
/***********************************/
// stores the routing counter at its position d6-d4 in the EIB frame.
uint8_t eib_default_route_counter; 

/**
* @brief returns the default routing counter inserted by the Network Layer
*/
uint8_t eib_get_route_counter (void)
{
	return eib_default_route_counter >> 4;
}

/**
* @brief sets the default routing counter inserted by the Network Layer
*/
void eib_set_route_counter (uint8_t new_counter)
{
	eib_default_route_counter = (new_counter & 0x07) << 4;
}

/**
* @brief copies message into transmission buffer. Returns 1, if ok; returns 0, if buffer was full
*/
char eib_N_DATA_request(t_eib_frame* msg) {
	// insert the routing counter

	((t_eib_message *)&(msg->frame))->NPCI |= eib_default_route_counter;

/*int i;
	for (i = 0; i < msg->len; i++)
		printf_P(PSTR("%2.2X "), msg->frame[i]);
	printf_P(PSTR("\n"));
*/
	return eib_L_DATA_request(msg, EIB_DEVICE_CHANNEL);
}

/**
* @brief retrieves message from reception buffer. Returns 1, if ok; returns 0, if buffer was empty
*/
char eib_N_DATA_indication_poll (t_eib_frame* msg) {
	return eib_L_DATA_indication_poll (msg);
}
/**
* @brief retrieves message from reception buffer. Waits until message is available
*/
void eib_N_DATA_indication_wait (t_eib_frame* msg) {
	return eib_L_DATA_indication_wait (msg);
}



/**
* @brief Sends group message. Returns 1, if ok; returns 0, if buffer was full
* address: group address
* *data: pointer to transmit data
* len: len of transmit data: 0=0..6 bit, 1=1byte, 2=2byte, etc
*/
char eib_G_DATA_request(uint16_t address, uint8_t *data, uint8_t len) {

t_eib_frame msg;

#ifdef EIB_VIRTUAL_MSG_SUPPORT
	// virtual messages are queued internally
	if (((address >> 3) & 0x1f) > MAX_EIB_MAIN_GROUP) {
		eib_virtual_queue_msg (address, data, len);
		return 1;
	}
#endif

	((t_eib_message*)&(msg.frame))->ctrl = 0xBC;
	((t_eib_message*)&(msg.frame))->destination = address;
	((t_eib_message*)&(msg.frame))->NPCI = 0x80 | ((len+1) & 0x0f);
	((t_eib_message*)&(msg.frame))->TPCI = 0x00;
	((t_eib_message*)&(msg.frame))->TSDU = 0x80;

	if (len) {
		memcpy( &(((t_eib_message*)&(msg.frame))->TSDU) +1, data, len);
	}
	else {
		((t_eib_message*)&(msg.frame))->TSDU |= (uint8_t)*data & 0x3f;
	}
	//set message length
	msg.len = len + 8;
	// insert the routing counter
	return eib_N_DATA_request (&msg);
}

/********************************/
/* Layer 2 (Link Layer) support */
/********************************/

/**
 * @brief EIB Link Layer receive service thread
 *
 * The endless loop in this thread waits for a new EIB message from Link Layer
 * and processes it.
 *
 */
THREAD(EIB_NL_Service, arg)
{

t_eib_frame msg;
uint16_t	source, dest;
uint8_t		len, apci;
uint8_t		*data;

    NutThreadSetPriority(NUT_THREAD_PRIORITY_EIB_LL_SERVICE);
    /*
     * Now loop endless for new EIB messages
     */
    for (;;) {
		eib_N_DATA_indication_wait (&msg);

//		printf_P (PSTR("src:%4.4x dst:%4.4x len:%2i\n"), ((t_eib_message*)&(msg.frame[0]))->source,
//														((t_eib_message*)&(msg.frame[0]))->destination,
//														msg.len);
		source = ((t_eib_message*)&(msg.frame))->source;
		// show message to busmon (if active)
		busmon_show (&msg);

		// check, if frame is a group message
		if (msg.frame[5] & 0x80) {
			// process group message

			//extract group message data
			dest = ((t_eib_message*)&(msg.frame))->destination;
			len = (((t_eib_message*)&(msg.frame))->NPCI & 0x0f) -1;
			apci = (((t_eib_message*)&(msg.frame))->TPCI & 0x03) << 2 | (((t_eib_message*)&(msg.frame))->TSDU & 0xC0) >> 6;

			if (len)
				data = &(((t_eib_message*)&(msg.frame))->TSDU) +1;
			else {
				((t_eib_message*)&(msg.frame))->TSDU &= 0x3f;
				data = &(((t_eib_message*)&(msg.frame))->TSDU);
			}
			// forward message to object layer functions
			if (eib_objects_process_msg (dest, data, len, apci)) {
				// forward message to lcd functions
				lcd_listen_process_msg (dest);
				lcd_page_process_msg (dest);
				lcd_listen_process_msg (dest);
			}
/*
			dest = ((t_eib_message*)&(msg.frame))->destination;
			inttostr(dest,ss);
			showzifustr(50,50,"Addr:",0xf800,0xffff);
			showzifustr(100,50,ss,0xf800,0xffff);	//��ʾ�ַ� 
			inttostr(*data,ss);
			showzifustr(150,50," = ",0xf800,0xffff);
			showzifustr(180,50,ss,0xf800,0xffff);	//��ʾ�ַ� 
*/
		}
		else {
			// process message with device address
			dest = ((t_eib_message*)&(msg.frame))->destination;
			if (dest == eib_get_device_address(EIB_DEVICE_CHANNEL)) {
				// we are addressed
				eib_TL_data_indication (&msg);
			}
		}
	}
}


/**
* @brief init of the EIB network layers
*
* This function calls the init of TPUART and initializes all functions to operate
* the Network, transport and application layers.
*/
void init_eib_layers (void)
{
	eib_tl_state = CLOSED;
	// register thread to dispatch messages from Link Layer
	NutThreadCreate("EIBNLsrv", EIB_NL_Service, 0, NUT_THREAD_EIBSERVICE_STACK);
	// register thread for TL state machine
	NutThreadCreate("EIBTLsrv", EIB_TL_Service, 0, NUT_THREAD_EIBSERVICE_STACK);
	// init the TPUART Link Layer driver
	eib_control (EIB_INIT_CMD);
	// init the physical address
	init_physical_address_from_Flash ();
	// init routing counter
	eib_set_route_counter (EIB_DEFAULT_ROUTING_COUNTER);
}

/**
* @brief function for LL driver to check, if device is addressed
*
* NPCI = 0x80: group address, =0x00: phys. address
* addr = EIB address
* returns 0, if device is not addressed
* returns 1, if device is addressed
*/
unsigned char eib_check_group_address (uint16_t addr) {

	if (get_group_adress_index (addr) < 0)
		return 0;
	return 1;
}

