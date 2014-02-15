/** \file EIBLayers.h
 *  \brief Constants and definitions for EIB protocol stack layers above LL
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef EIB_LAYERS_H_
#define EIB_LAYERS_H_

// this is the highest "real" group address allowed on EIB
#define	MAX_EIB_MAIN_GROUP		15	
// this is the highest group address allowed in Timer and Logic.
// Addresses between MAX_EIB_MAIN_GROUP and MAX_LOGIC_MAIN_GROUP are valid only 
// inside the Home Director Device. They are not transmitted to EIB or Ethernet.
#define MAX_LOGIC_MAIN_GROUP	31	

enum _EIB_TL_STATES { CLOSED, OPEN_IDLE, OPEN_WAIT_FOR_T_DATA_ACK };
typedef enum _EIB_TL_STATES EIB_TL_STATES;

typedef struct {
uint8_t	ctrl;			// d7-d6: frame format: 00: long length, 10: std. length, 11: polling
						// d5=1: no repetition, =1: repeated message
						// d4: frame type = 1
						// d3-d2: priority: 00=system, 10=alarm, 01=high, 11=low
						// d1-d0: preamble, fix 00
uint16_t	source;		// source address, HB/LB!
uint16_t	destination;// destination address, HB/LB!
uint8_t		NPCI;		// d7=0: phys address, =1: group address
						// d6-d4: routing counter
						// d3-d0: data length (std. message only)
uint8_t		TPCI;		// d7-d6: TPDU type: 	00=data packet(UDT), 
						//						01=numbered data packet(NDT)
						//						10=control data(UCD)
						//						11=numbered control data(NCD)
						// d5-d2: TPDU=UDT: don't care (=0000)
						//        TPDU=NDT: sequence number
						//        TPDU=UCD: don't care (=0000)
						//        TPDU=NCD: sequence number
						// d1-d0: TPDU=UDT: TSDU
						//		  TPDU=NDT: TSDU
						//		  TPDU=UCD: control:00=open, 01=close
						//		  TPDU=NCD: control: 10=ACK, 11=NAK
uint8_t		TSDU;

} t_eib_message;


//virtual device channel used by device functions
#define EIB_DEVICE_CHANNEL	0
//virtual device channel used by EIBNet/IP functions
#define EIB_EIBNET_CHANNEL	1
// default value for max transfer count for Network Layer
#define EIB_DEFAULT_ROUTING_COUNTER		6
// EIB service primitive numbers
#define L_DATA_REQUEST		0x11
#define L_DATA_CONFIRM		0x4E
#define L_DATA_INDICATION	0x49

// length of AL reply buffer
#define EIB_AL_DATA_LEN		0x10

// define emulated BCU mask is 1.2
#define DEVICE_MASK_TYPE			0x00
#define DEVICE_MASK_VERSION			0x12

// TL primitives
#define TL_CTRL_MSG_LEN				7
#define TPDU_POSITION				6
#define APCI_POSITION				7
#define TPDU_FULL_MASK				0xC3
#define TPDU_MASK					0xC0
#define TPDU_NUMBERED_DATA			0x40
#define TPDU_OPEN_CONNECTION		0x80
#define TPDU_CLOSE_CONNECTION		0x81
#define	TPDU_SEQUENCE_OFFSET		2
#define TPDU_SEQUENCE_MASK			0x0F
#define TPDU_ACK_DATA				0xC2
#define TPDU_NACK_DATA				0xC3
#define TL_MAX_MSG_TRIES			5
#define	TPDU_UDT					0x00
#define TPDU_NDT					0x40
#define TPDU_UCD					0x80
#define TPDU_NCD					0xC0

#define	A_READ_MASK_VERSION_REQ_PDU	0x300
#define	A_READ_MASK_VERSION_RES_PDU	0x340
#define	A_READ_MEM_REQ_PDU			0x200
#define A_MEM_MASK					0xFFF0 // mask for memory APCI
#define A_READ_MEM_LEN_MASK			0x0F // mask for len in APCI
#define	A_READ_MEM_RES_PDU			0x240
#define A_ADC_MASK					0xFFC0 // mask for ADC read
#define A_ADC_CHANNEL_MASK			0x3F // mask for ADC channel
#define A_READ_ADC_REQ_PDU			0x180
#define A_READ_ADC_RES_PDU			0x1C0

// frame positions of source address
#define EIB_SRC_ADDRESS_HIGH		1
#define EIB_SRC_ADDRESS_LOW			2
#define EIB_DEST_ADDRESS_HIGH		3
#define EIB_DEST_ADDRESS_LOW		4

// TL timeout
#define EIB_TL_CONNECTION_TIMEOUT	6000	// 6000ms timeout
#define EIB_TL_ACKNOWLEDGE_TIMEOUT	3000	// 3000ms timeout
#define EIB_TL_TIMEOUT_INTERVAL		100		// 100ms timer

// AL memory emulation
#define MADDR_STATUS_BYTE		0x60
#define MADDR_BCU_DATA_BYTE_0	0x101	
#define MADDR_BCU_DATA_BYTE_1	0x102	
#define MADDR_BCU_DATA_BYTE_2	0x103	
#define MADDR_MANUF_BYTE		0x104	

// AL constants
#define APCI_VALUE_READ			0x00
#define APCI_VALUE_RESPONSE		0x01
#define APCI_VALUE_WRITE		0x02

// interface to external modules
uint8_t eib_get_route_counter (void);
void eib_set_route_counter (uint8_t);

// init the EIB stack
void init_eib_layers (void);

//copies message into transmission buffer. Returns 1, if ok; returns 0, if buffer was full
char eib_N_DATA_request(t_eib_frame*);

//retrieves message from reception buffer. Returns 1, if ok; returns 0, if buffer was empty
char eib_N_DATA_indication_poll (t_eib_frame*);
//retrieves message from reception buffer. Waits until message is available
void eib_N_DATA_indication_wait (t_eib_frame*);
// check, if group address should be acknowledged on the EIB
unsigned char eib_check_group_address (uint16_t);
// request EIB group message
char eib_G_DATA_request(uint16_t, uint8_t*, uint8_t);

#endif // EIB_LAYERS_H_
