/** \file ds1820.c
 *  \brief Functions for DS18x20 communication
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- init and reset GPIO for DS18x20 communication
 *	- state machine communicating to DS18x20 temperature sensors
 *	- transformation of raw data to IEEE floating point value
 *	- transformation of floating point value into EIS5 value
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "1wire_io.h"
#include "ds1820.h"
#include "hardware.h"
#include <util/crc16.h>

enum DS1820_STATES { DS1820_IDLE, DS1820_START_CONVERSION, DS1820_END_OF_CONVERSION, DS1820_READ0, DS1820_READ1, DS1820_READ2, DS1820_READ3,
							DS1820_READ4, DS1820_READ5, DS1820_READ6, DS1820_READ7, DS1820_READ8 };
// state machine states for all HW channels
enum DS1820_STATES ds1820_state[sizeof(port_bit)];
uint16_t	ds1820_timer[sizeof(port_bit)];
uint8_t ds1820_crc[sizeof(port_bit)];
double ds1820_temp[sizeof(port_bit)];
uint8_t ds1820_remain[sizeof(port_bit)];

//final static String repeatTimes[] = { "1s", "2s", "5s", "10s", "30s", "60s" };
const uint16_t DS1820_REPEAT_TIMES[] = { 33, 65, 152, 303, 909, 1818 };

void init_ds1820 (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;

	init_1wire (p->hw_channel);
	// set state machine to wait for next conversion task
	ds1820_state [p->hw_channel] = DS1820_IDLE;
	ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

}

void terminate_ds1820 (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;

	terminate_1wire (p->hw_channel);

}


uint8_t c;
uint16_t b;
int16_t w;
double r;


// this is the cyclic tick for the DS18S20 state machine
void ds18S20_statemachine (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;
	c = p->hw_channel;
	if (c > sizeof(port_bit))
		return;

	if (ds1820_timer[c] > 0)
		ds1820_timer[c]--;
	if (ds1820_timer[c] > 0)
		return;

	switch (ds1820_state[c]) {

		case DS1820_IDLE:
			// set reset request
			if (reset_1wire (c, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, c, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("No Slave %d\n"), c);
#endif
				break;
			}
			// issue conversion start
			ds1820_state [p->hw_channel] = DS1820_START_CONVERSION;
#ifdef HW_DEBUG
    printf_P(PSTR("Slave %d ok\n"), c);
#endif
		break;
		case DS1820_START_CONVERSION:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, c);
			send_1wire_byte (DS1820_CMD_CONVERT, c);
			ds1820_state [p->hw_channel] = DS1820_END_OF_CONVERSION;
			ds1820_timer [p->hw_channel] = DS1820_CONVERSION_TICKS;
#ifdef HW_DEBUG
    printf_P(PSTR("Conversion %d\n"), c);
#endif
		break;
		case DS1820_END_OF_CONVERSION:
			// set reset request
			if (reset_1wire (c, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];
				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, c, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("No Slave %d\n"), c);
#endif
				break;
			}
			// issue conversion start
			ds1820_state [p->hw_channel] = DS1820_READ0;
#ifdef HW_DEBUG
    printf_P(PSTR("Slave %d ok\n"), c);
#endif
		break;
		case DS1820_READ0:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, c);
			send_1wire_byte (DS1820_CMD_READ, c);
			ds1820_crc[c] = 0;
			b = receive_1wire_byte(c);
			ds1820_temp[c] = b;
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ1;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ1:
			b = receive_1wire_byte(c);
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
			w= ds1820_temp[c];
			w |= (int16_t) (b << 8);
			ds1820_temp[c] = w;
			ds1820_temp[c] *= 0.5;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %4.4x = %2.2f C\n"), w, ds1820_temp[c]);
#endif
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ2;
		break;
		case DS1820_READ2:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ3;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ3:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ4;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ4:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ5;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ5:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ6;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ6:
			b = receive_1wire_byte(c);
			// remember for remainder calculation
			ds1820_remain[c] = b;
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ7;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ7:
			b = receive_1wire_byte(c);
			// this is count per C
			// add remainder

			r = b - ds1820_remain[c];
			ds1820_temp[c] += (-0.25 + r/b);

#ifdef HW_DEBUG
    printf_P(PSTR("With remainder %2.2f C\n"), ds1820_temp[c]);
#endif

			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ8;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ8:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);

#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d with CRC=%2.2x\n"), b, c, ds1820_crc[c]);
#endif

			ds1820_state [p->hw_channel] = DS1820_IDLE;
			ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

			// add temperature offset
			r = p->temp_offset;
			ds1820_temp[c] += r/10;
#ifdef HW_DEBUG
    printf_P(PSTR("With offset %2.2f C\n"), ds1820_temp[c]);
#endif

			// forward to HW monitor
			hwmon_show_ds1820_event (ds1820_temp[c], r, c, ds1820_crc[c], 1);

			// check CRC
			if (!ds1820_crc[c]) {

				// convert value and sent it to bus
				switch (p->eis_number_format) {
					case DS1820_FORMAT_EIS5:

						ds1820_temp[c] *= 100.0/8.0;
						w = (int16_t) ds1820_temp[c];
						w &= 0x07ff;
						w |= (3 << 11);
						if (ds1820_temp[c] < 0)
							w |= 0x8000;
						// exchange LB-HB
						b = (w >> 8) & 0xff;
						w = w << 8;
						w |= b;
						b = p->eib_object;
						eib_G_DATA_request(get_group_address (b), (uint8_t*)&w, 2);
					break;
				}

			}
			else {
				// next conversion after 1 sec.
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [0];
			}


		break;

		// should never happen
		default: ds1820_state[c] = DS1820_IDLE;
#ifdef HW_DEBUG
    printf_P(PSTR("ERROR: default state hit for %d\n"), c);
#endif
	}
}


// this is the cyclic tick for the DS18B20 state machine
void ds18B20_statemachine (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;
	c = p->hw_channel;
	if (c > sizeof(port_bit))
		return;

	if (ds1820_timer[c] > 0)
		ds1820_timer[c]--;
	if (ds1820_timer[c] > 0)
		return;

	switch (ds1820_state[c]) {

		case DS1820_IDLE:
			// set reset request
			if (reset_1wire (c, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, c, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("No Slave %d\n"), c);
#endif
				break;
			}
			// issue conversion start
			ds1820_state [p->hw_channel] = DS1820_START_CONVERSION;
#ifdef HW_DEBUG
    printf_P(PSTR("Slave %d ok\n"), c);
#endif
		break;
		case DS1820_START_CONVERSION:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, c);
			send_1wire_byte (DS1820_CMD_CONVERT, c);
			ds1820_state [p->hw_channel] = DS1820_END_OF_CONVERSION;
			ds1820_timer [p->hw_channel] = DS1820_CONVERSION_TICKS;
#ifdef HW_DEBUG
    printf_P(PSTR("Conversion %d\n"), c);
#endif
		break;
		case DS1820_END_OF_CONVERSION:
			// set reset request
			if (reset_1wire (c, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];
				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, c, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("No Slave %d\n"), c);
#endif
				break;
			}
			// issue data read start
			ds1820_state [p->hw_channel] = DS1820_READ0;
#ifdef HW_DEBUG
    printf_P(PSTR("Slave %d ok\n"), c);
#endif
		break;
		case DS1820_READ0:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, c);
			send_1wire_byte (DS1820_CMD_READ, c);
			ds1820_crc[c] = 0;
			b = receive_1wire_byte(c);
			ds1820_temp[c] = b;
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ1;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ1:
			b = receive_1wire_byte(c);
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
			w= ds1820_temp[c];
			w |= b << 8;
			ds1820_temp[c] = w * 0.0625; // 12 bit resolution
#ifdef HW_DEBUG
    printf_P(PSTR("Read %4.4x = %2.2f C\n"), w, ds1820_temp[c]);
#endif
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ2;
		break;
		case DS1820_READ2:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ3;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ3:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ4;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ4:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ5;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ5:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ6;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ6:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ7;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ7:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);
			ds1820_state [p->hw_channel] = DS1820_READ8;
#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d\n"), b, c);
#endif
		break;
		case DS1820_READ8:
			b = receive_1wire_byte(c);
			ds1820_crc[c] = _crc_ibutton_update (ds1820_crc[c], b);

#ifdef HW_DEBUG
    printf_P(PSTR("Read %2.2x from %d with CRC=%2.2x\n"), b, c, ds1820_crc[c]);
#endif

			ds1820_state [p->hw_channel] = DS1820_IDLE;
			ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

			// add temperature offset
			r = p->temp_offset;
			ds1820_temp[c] += r/10;
#ifdef HW_DEBUG
    printf_P(PSTR("With offset %2.2f C\n"), ds1820_temp[c]);
#endif

			// forward to HW monitor
			hwmon_show_ds1820_event (ds1820_temp[c], r, c, ds1820_crc[c], 1);

			// check CRC
			if (!ds1820_crc[c]) {

				// convert value and sent it to bus
				switch (p->eis_number_format) {
					case DS1820_FORMAT_EIS5:

						ds1820_temp[c] *= 100.0/8.0;
						w = (int16_t) ds1820_temp[c];
						w &= 0x07ff;
						w |= (3 << 11);
						if (ds1820_temp[c] < 0)
							w |= 0x8000;
						// exchange LB-HB
						b = (w >> 8) & 0xff;
						w = w << 8;
						w |= b;
						b = p->eib_object;
						eib_G_DATA_request(get_group_address (b), (uint8_t*)&w, 2);
					break;
				}

			}
			else {
				// next conversion after 1 sec.
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [0];
			}


		break;

		// should never happen
		default: ds1820_state[c] = DS1820_IDLE;
#ifdef HW_DEBUG
    printf_P(PSTR("ERROR: default state hit for %d\n"), c);
#endif
	}
}
