/** \file dht11.c
 *  \brief Functions for DHT11 communication
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- init and reset GPIO for DHT11 communication
 *	- state machine communicating to DHT11 temperature/humidity sensors
 *	- transformation of raw data to IEEE floating point values
 *	- transformation of floating point value into EIS5 (9.001 and 9.007) values
 *
 *	Copyright (c) 2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *  Version 0.9
 */
#include "1wire_io.h"
#include "dht11.h"
#include "hardware.h"
#include <math.h>	// Dew Point calculation

enum DHT_STATES { DHT_IDLE, DHT_READ, DHT_CONVERT, DHT_SEND};
// state machine states for all HW channels
enum DHT_STATES dht_state[sizeof(port_bit)];
uint16_t dht_timer[sizeof(port_bit)];
uint8_t dht_crc[sizeof(port_bit)];
double dht_temp[sizeof(port_bit)];
double dht_humid[sizeof(port_bit)];
double dht_dew[sizeof(port_bit)];

// Used to store read DHT 11 bytes, pointer to read function
uint8_t dht_byte_read[sizeof(port_bit)][5];
uint8_t dht_read_ok[sizeof(port_bit)];

//final static String repeatTimes[] = { "1s", "2s", "5s", "10s", "30s", "60s" };
const uint16_t DHT_REPEAT_TIMES[] = { 33, 65, 152, 303, 909, 1818 };
const char dht_names[4][3] = { "11", "21", "22", "03" };


// Help Functions for Dew Point calculation #############################################

// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double dewPoint(double celsius, double humidity)
{
	// (1) Saturation Vapor Pressure = ESGG(T)
	double RATIO = 373.15 / (273.15 + celsius);
	double RHS = -7.90298 * (RATIO - 1);
	RHS += 5.02808 * log10(RATIO);
	RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
	RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
	RHS += log10(1013.246);

	// factor -3 is to adjust units - Vapor Pressure SVP * humidity
	double VP = pow(10, RHS - 3) * humidity;

	// (2) DEWPOINT = F(Vapor Pressure)
	double T = log(VP/0.61078);   // temp var
	return (241.88 * T) / (17.558 - T);
}

// delta max = 0.6544 wrt dewPoint()
// 6.9 x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
	double a = 17.271;
	double b = 237.7;
	double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
	double Td = (b * temp) / (a - temp);
	return Td;
}
// ######################################################################################


void init_dht (char *cp) {

_O_DHTxx_t*	p;

	// Local copy
	p = (_O_DHTxx_t*) cp;

	init_1wire (p->hw_channel);
	// set state machine to wait for next conversion task
	dht_state [p->hw_channel] = DHT_IDLE;
	dht_timer [p->hw_channel] = DHT_REPEAT_TIMES [p->repeat_frequency];
}

void terminate_dht (char *cp) {

_O_DHTxx_t*	p;

	p = (_O_DHTxx_t*) cp;

	terminate_1wire (p->hw_channel);
}


uint8_t s;	// sensor type
uint8_t c;	// channel
uint16_t b;
int16_t w;
double r;


// this is the cyclic tick for the DHT state machine
//
// DHT 11 Reset pulse minimum 18ms -> handeled by own state
// --> Blocking time approx. 4ms
// DHT 2x Reset is time critical 0.8-20ms -> handeled by reading state
// --> Blocking time approx. 5ms
void dht_statemachine (char *cp) {

_O_DHTxx_t*	p;

	p = (_O_DHTxx_t*) cp;
	c = p->hw_channel;
	s = p->sensor_type;
	if (c > sizeof(port_bit))
		return;

	if (dht_timer[c] > 0)
	dht_timer[c]--;
	if (dht_timer[c] > 0)
		return;

	switch (dht_state[c]) {

		case DHT_IDLE:
			// Do we support the sensor
			if ( s>=(sizeof(dht_names)/sizeof(dht_names[0]))) {
				hwmon_show_dht_event(0, 0, 0, 0, 0, c, 0, 255, 0);		// Sensor not supported
				// Reprint message after 60s
				dht_timer [c] = DHT_REPEAT_TIMES [5];
				break;
			}
			if (s==DHT11_SENSOR)
				reset_1wire(c, DHT_11_1WIRE);
			dht_state [c] = DHT_READ;
#ifdef HW_DEBUG
	printf_P(PSTR("DHT%s Start send, sensor type %u\n"), dht_names[s], s);
#endif
			break;

		case DHT_READ:
			dht_state [c] = DHT_CONVERT;
			if (s==DHT21_SENSOR || s==DHT22_SENSOR)
				reset_1wire(c, DHT_2x_1WIRE);
			dht_read_ok[c] = receive_1wire_dht11(c, dht_byte_read[c]);
			break;

		case DHT_CONVERT:
			dht_state [c] = DHT_SEND;
#ifdef HW_DEBUG
	uint8_t i;
	if (!dht_read_ok[c])
		printf_P(PSTR("DHT Slave %u OK\n"), c);
	else
		printf_P(PSTR("DHT Slave %u Timeout\n"), c);
	// Dump all bytes
	for (i = 0; i<5; i++) {
		printf_P(PSTR("DHT Slave %u Byte %u = %u\n"), c, i, dht_byte_read[c][i]);
	}
#endif
			// Fault if not 0
			if (dht_read_ok[c]) {
				dht_state [c] = DHT_IDLE;
				dht_timer [c] = DHT_REPEAT_TIMES [p->repeat_frequency];
				// forward to HW monitor
				hwmon_show_dht_event(0, 0, 0, 0, 0, c, 0, dht_read_ok[c], s);
				break;	// Fault restart again
			}

			// Calculate own checksum
			if (dht_byte_read[c][4] == ( (dht_byte_read[c][0]+dht_byte_read[c][1]+dht_byte_read[c][2]+dht_byte_read[c][3]) &0xFF) )
				dht_crc[c] = 0;	//OK
			else
				dht_crc[c] = dht_byte_read[c][4];	//Failure

			// Calculate Humidity, Temperature and Dew Point
			// Convert into double
			if (s==DHT11_SENSOR) {
					dht_humid[c] = dht_byte_read[c][0];	// Humidity DHT11

					// Handle negative temperatures
					if (dht_byte_read[c][2] & 0x80) {
						dht_temp[c] = ((0x7F & dht_byte_read[c][2]) *-1);
					}
					else {
						dht_temp[c] = dht_byte_read[c][2];
					}
			}
			else {	// DHT2x Sensors
				dht_humid[c] = ( (dht_byte_read[c][0]<<8) + dht_byte_read[c][1]) * 0.1;	// Humidity

				// Handle negative temperatures
				if (dht_byte_read[c][2] & 0x80) {
					dht_temp[c] = ( ((0x7F & dht_byte_read[c][2])<<8) + dht_byte_read[c][3]) * -0.1;
				}
				else {
					dht_temp[c] = ( (dht_byte_read[c][2]<<8) + dht_byte_read[c][3]) * 0.1;
				}
			}

			// add temperature offset
			r = p->temp_offset;
			dht_temp[c] += r/10;

			// add humidity offset
			r = p->humid_offset;
			dht_humid[c] += r/10;


			// Calculate the Dew Point
			dht_dew[c] = dewPointFast(dht_temp[c], dht_humid[c]);

#ifdef HW_DEBUG
	printf_P(PSTR("With offset: T %5.1f C(%d)   H %5.1f %%(%d) --> Dew %5.1f C\n"), dht_temp[c], p->temp_offset, dht_humid[c], p->humid_offset, dht_dew[c]);
#endif

			// forward to HW monitor
			hwmon_show_dht_event(dht_temp[c], dht_humid[c], dht_dew[c], p->temp_offset, p->humid_offset, c, dht_crc[c], dht_read_ok[c], s);
			break;

		case DHT_SEND:
			dht_state [c] = DHT_IDLE;
			dht_timer [c] = DHT_REPEAT_TIMES [p->repeat_frequency];
			// check CRC
			if (!dht_crc[c]) {

				// convert value and sent it to bus
				switch (p->eis_number_format) {
					case DHT11_FORMAT_EIS5:

					// Temperature
					dht_temp[c] *= 100.0/8.0;
					w = (int16_t) dht_temp[c];
					w &= 0x07ff;
					w |= (3 << 11);
					if (dht_temp[c] < 0)
					w |= 0x8000;
					// exchange LB-HB
					b = (w >> 8) & 0xff;
					w = w << 8;
					w |= b;
					b = p->eib_object;		// temperature address
					eib_G_DATA_request(get_group_address (b), (uint8_t*)&w, 2);

					// Humidity
					dht_humid[c] *= 100.0/8.0;
					w = (int16_t) dht_humid[c];
					w &= 0x07ff;
					w |= (3 << 11);
					if (dht_humid[c] < 0)
					w |= 0x8000;
					// exchange LB-HB
					b = (w >> 8) & 0xff;
					w = w << 8;
					w |= b;
					XRAM_SELECT_BLOCK(XRAM_CYCLIC_ELEMENTS_PAGE);	// Reselect, lost after get_group_address()
					b = p->eib_object2;		// humidity address
					eib_G_DATA_request(get_group_address (b), (uint8_t*)&w, 2);
					break;
				}
			}
			else {
				// Failed, restart new conversion after 2 sec.
				dht_timer [c] = DHT_REPEAT_TIMES [1];
			}
			break;

		// should never happen
		default: dht_state[c] = DHT_IDLE;
#ifdef HW_DEBUG
	printf_P(PSTR("ERROR: default state hit for %d\n"), c);
#endif
	}
}
