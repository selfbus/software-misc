/** \file e_value.c
 *  \brief Functions for value element
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- display the number value of an EIB Object
 *	- use symbols stored as icons in the Nand Flash
 *	- change displayed value on reception of EIB messages
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "e_value.h"
#include "math.h"
#include "System.h"

uint16_t show_value_string (uint16_t ofs_0, uint16_t x1, uint16_t y1, char *str) {
	while(*str!='\0')
	{	
		if ((*str >= '0') && (*str <= '9')) {
			x1 += draw_picture (ofs_0 + (*str - '0'), x1, y1);
		}
		else if (*str == '.') {
			x1 += draw_picture (ofs_0 + CHAR_OFFSET_DOT, x1, y1);
		}
		else if (*str == '-') {
			x1 += draw_picture (ofs_0 + CHAR_OFFSET_MINUS, x1, y1);
		}
		else if (*str == '+') {
			x1 += draw_picture (ofs_0 + CHAR_OFFSET_PLUS, x1, y1);
		}
		else if (*str == ':') {
			x1 += draw_picture (ofs_0 + CHAR_OFFSET_DP, x1, y1);
		}
		else {
			x1 += get_picture_width (ofs_0 + CHAR_OFFSET_MINUS);
		}

		str++;
	}
	return x1;
}

uint16_t show_value_string_90L (uint16_t ofs_0, uint16_t x1, uint16_t y1, char *str) {
	while(*str!='\0')
	{	
		if ((*str >= '0') && (*str <= '9')) {
			y1 += draw_picture_y (ofs_0 + (*str - '0'), x1, y1);
		}
		else if (*str == '.') {
			y1 += draw_picture_y (ofs_0 + CHAR_OFFSET_DOT, x1, y1);
		}
		else if (*str == '-') {
			y1 += draw_picture_y (ofs_0 + CHAR_OFFSET_MINUS, x1, y1);
		}
		else if (*str == '+') {
			y1 += draw_picture_y (ofs_0 + CHAR_OFFSET_PLUS, x1, y1);
		}
		else if (*str == ':') {
			y1 += draw_picture_y (ofs_0 + CHAR_OFFSET_DP, x1, y1);
		}
		else {
			y1 += get_picture_height (ofs_0 + CHAR_OFFSET_MINUS);
		}

		str++;
	}
	return y1;
}

uint16_t show_value_string_90R (uint16_t ofs_0, uint16_t x1, uint16_t y1, char *str) {
	while(*str!='\0')
	{	
		if ((*str >= '0') && (*str <= '9')) {
			y1 -= get_picture_height (ofs_0 + (*str - '0'));
			draw_picture (ofs_0 + (*str - '0'), x1, y1);
		}
		else if (*str == '.') {
			y1 -= get_picture_height (ofs_0 + CHAR_OFFSET_DOT);
			draw_picture (ofs_0 + CHAR_OFFSET_DOT, x1, y1);
		}
		else if (*str == '-') {
			y1 -= get_picture_height (ofs_0 + CHAR_OFFSET_MINUS);
			draw_picture (ofs_0 + CHAR_OFFSET_MINUS, x1, y1);
		}
		else if (*str == '+') {
			y1 -= get_picture_height (ofs_0 + CHAR_OFFSET_PLUS);
			draw_picture (ofs_0 + CHAR_OFFSET_PLUS, x1, y1);
		}
		else if (*str == ':') {
			y1 -= get_picture_height (ofs_0 + CHAR_OFFSET_DP);
			draw_picture (ofs_0 + CHAR_OFFSET_DP, x1, y1);
		}
		else {
			y1 -= get_picture_height (ofs_0 + CHAR_OFFSET_MINUS);
			get_picture_height (ofs_0 + CHAR_OFFSET_MINUS);
		}

		str++;
	}
	return y1;
}


uint16_t show_value_string_180 (uint16_t ofs_0, uint16_t x1, uint16_t y1, char *str) {
	while(*str!='\0')
	{	
		if ((*str >= '0') && (*str <= '9')) {
			x1 -= get_picture_width (ofs_0 + (*str - '0'));
			draw_picture (ofs_0 + (*str - '0'), x1, y1);
		}
		else if (*str == '.') {
			x1 -= get_picture_width (ofs_0 + CHAR_OFFSET_DOT);
			draw_picture (ofs_0 + CHAR_OFFSET_DOT, x1, y1);
		}
		else if (*str == '-') {
			x1 -= get_picture_width (ofs_0 + CHAR_OFFSET_MINUS);
			draw_picture (ofs_0 + CHAR_OFFSET_MINUS, x1, y1);
		}
		else if (*str == '+') {
			x1 -= get_picture_width (ofs_0 + CHAR_OFFSET_PLUS);
			draw_picture (ofs_0 + CHAR_OFFSET_PLUS, x1, y1);
		}
		else if (*str == ':') {
			x1 -= get_picture_width (ofs_0 + CHAR_OFFSET_DP);
			draw_picture (ofs_0 + CHAR_OFFSET_DP, x1, y1);
		}
		else {
			x1 -= get_picture_width (ofs_0 + CHAR_OFFSET_MINUS);
		}

		str++;
	}
	return x1;
}

void show_value_string_with_postfix (_E_VALUE_t* p, char *str) {

uint16_t post_pict = p->picture_index1 + PICTURE_OFFSET_POSTFIXUNIT;
uint16_t ofs_0 = p->picture_index1 + PICTURE_OFFSET_ZERO;
uint16_t x1 = p->x_pos;
uint16_t y1 = p->y_pos;
uint16_t tdx = p->text_x;

	if (display_orientation == DISPLAY_ORIENTATION_HOR) {
		draw_picture (post_pict, show_value_string (ofs_0, x1+tdx, y1, str), y1);
	}
	else if (display_orientation == DISPLAY_ORIENTATION_90L) {
		draw_picture (post_pict, x1, show_value_string_90L (ofs_0, x1, y1+tdx, str));
	}
	else if (display_orientation == DISPLAY_ORIENTATION_90R) {
		draw_picture (post_pict, x1, show_value_string_90R (ofs_0, x1, y1-tdx+get_picture_height (p->picture_index1 + PICTURE_OFFSET_BACKGROUND), str) - get_picture_height (post_pict));
	}
	else if (display_orientation == DISPLAY_ORIENTATION_UPSIDE) {
		draw_picture (post_pict, show_value_string_180 (ofs_0, x1-tdx+get_picture_width (p->picture_index1 + PICTURE_OFFSET_BACKGROUND), y1, str)- get_picture_width (post_pict), y1);
	}
}

void draw_value_element (char* cp) {

_E_VALUE_t*	p;
uint16_t val;
char numstr[MAX_VALUE_LENGTH];
uint8_t integers;
uint8_t decimals;
float fval;
uint32_t *val32;
uint8_t td[4];

	p = (_E_VALUE_t*) cp;

	// put backgound image
	draw_picture (p->picture_index1 + PICTURE_OFFSET_BACKGROUND, p->x_pos, p->y_pos);

	// put value
	integers = (p->chars >> 4) & 0x0f;
	decimals = p->chars & 0x0f;

	// check object value type and calculate value string
	switch (p->parameter & 0x0f) {
		// EIS6, 0-100%
		case 0:
			val = eib_get_object_8_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			val &= 0xff;
			val *= 100;
			val /= 0xff;
			sprintf_P (numstr, PSTR("%*.*d"), integers, decimals, val);
		break;
		
		// EIS6, 0-255%
		case 1:
			val = eib_get_object_8_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("%*.*d"), integers, decimals, val & 0xff);
		break;

		// EIS5
		case 2:
			fval = eib_get_object_EIS5_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("% *.*f"), integers+decimals+2, decimals, fval);
		break;

		// EIS9
		case 3:
			val32 = (uint32_t*)&fval;
			*val32 = eib_get_object_32_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("% *.*f"), integers+decimals+2, decimals, fval);
		break;
		// EIS10: 16 bit unsigned
		case 4:
			val = eib_get_object_16_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("%*u"), integers, val);
		break;
		// EIS10: 16 bit signed
		case 5:
			val = eib_get_object_16_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("%*d"), integers, (int16_t)val);
		break;
		// EIS10: 32 bit unsigned
		case 6:
			val32 = (uint32_t*)td;
			*val32 = eib_get_object_32_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("%*lu"), integers, *val32);
		break;
		// EIS10: 32 bit signed
		case 7:
			val32 = (uint32_t*)td;
			*val32 = eib_get_object_32_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("%*ld"), integers, (int32_t)*val32);
		break;
		// EIS3: time
		case 8:
			val32 = (uint32_t*)td;
			*val32 = eib_get_object_32_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
				sprintf_P (numstr, PSTR("%02u:%02u"), td[3] & 0x1f, td[2]);
		break;
		// EIS4: date
		case 9:
			val32 = (uint32_t*)td;
			*val32 = eib_get_object_32_value (p->eib_object_listen);
			XRAM_SELECT_BLOCK(XRAM_PAGE_PAGE);
			sprintf_P (numstr, PSTR("%02u.%02u.20%02u"), td[3], td[2], td[1]);
		break;

		default:
			numstr[0] = '\0';
	}

	// output value to screen
	show_value_string_with_postfix (p, numstr);
}


void check_value_element (char* cp, int obj) {

_E_VALUE_t*	p;

	p = (_E_VALUE_t*) cp;

	if (p->eib_object_listen == obj)
		draw_value_element (cp);
}
