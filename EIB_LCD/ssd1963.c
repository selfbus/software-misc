#define SSD1963_TYPE_0	0		// 5.0" 800 X 480 display with SD Slot
#define SSD1963_TYPE_1	1		// 4.3" 480 x 272 display without SD Slot
#define SSD1963_TYPE_2	2		// 4.3" 480 x 272 display with SD Slot

#include <stdio.h>
#include <stdarg.h>
#include <avr/pgmspace.h>
#include "ssd1963.h"
#include "tft_io.h"

uint8_t current_lcd_type;	// Holds the currently used LCD type

extern int TP_X, TP_Y;	// Touch RAW value
extern int lx, ly;		// Scaled to screen resolution


// Initialize the SSD1963 LCD controller
// 0 = 5.0" 800x480
// 1 = 4.3" 480x272 without SD Slot
// 2 = 4.3" 480x272 with SD Slot (little brother of 5.0")
void ssd1963_controller_init(uint8_t lcd_type, uint8_t rotate, volatile uint16_t *lcd_res_x, volatile uint16_t *lcd_res_y) {

	switch (lcd_type) {
		case SSD1963_TYPE_0:
			#define		T0_HDP		799	//-->800
			#define		T0_HT		928
			#define		T0_HPS		46
			#define 	T0_LPS		15
			#define 	T0_HPW		48

			#define		T0_VDP		479	//-->480
			#define 	T0_VT		525
			#define		T0_VPS		16
			#define		T0_FPS		8
			#define		T0_VPW		16

			// Return used resolution
			current_lcd_type = SSD1963_TYPE_0;	// Set type for touch handling
			*lcd_res_x = T0_HDP;
			*lcd_res_y = T0_VDP;

			// enable wait
			XMCRA |= (1<<SRW00) | (1<<SRW01);// wait

			tft_set_pointer(SSD1963_set_pll_mn);//PLL multiplier, set PLL clock to 120M
			tft_write_byte(0x0023);//N=0x36 for 6.5M, 0x23 for 10M crystal
			tft_write_byte(0x0002);
			tft_write_byte(0x0004);
			tft_set_pointer(SSD1963_set_pll);// PLL enable
			tft_write_byte(0x0001);
			NutDelay(1);
			tft_set_pointer(SSD1963_set_pll);
			tft_write_byte(0x0003);
			NutDelay(5);
			tft_set_pointer(SSD1963_soft_reset);// software reset
			NutDelay(5);

			// disable wait
			XMCRA &= ~((1<<SRW00) | (1<<SRW01));// wait

			tft_set_pointer(SSD1963_set_lshift_freq);//PLL setting for PCLK, depends on resolution
			tft_write_byte(0x0003);
			tft_write_byte(0x00ff);
			tft_write_byte(0x00ff);

			tft_set_pointer(SSD1963_set_lcd_mode);//LCD SPECIFICATION
			tft_write_byte(0x0027);
			tft_write_byte(0x0000);
			tft_write_byte((T0_HDP>>8)&0X00FF);//Set HDP
			tft_write_byte(T0_HDP&0X00FF);
			tft_write_byte((T0_VDP>>8)&0X00FF);//Set VDP
			tft_write_byte(T0_VDP&0X00FF);
			tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_hori_period);//HSYNC
			tft_write_byte((T0_HT>>8)&0X00FF);//Set HT
			tft_write_byte(T0_HT&0X00FF);
			tft_write_byte((T0_HPS>>8)&0X00FF);//Set HPS
			tft_write_byte(T0_HPS&0X00FF);
			tft_write_byte(T0_HPW);//Set HPW
			tft_write_byte((T0_LPS>>8)&0X00FF);//Set HPS
			tft_write_byte(T0_LPS&0X00FF);
			tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_vert_period);//VSYNC
			tft_write_byte((T0_VT>>8)&0X00FF);//Set VT
			tft_write_byte(T0_VT&0X00FF);
			tft_write_byte((T0_VPS>>8)&0X00FF);//Set VPS
			tft_write_byte(T0_VPS&0X00FF);
			tft_write_byte(T0_VPW);//Set VPW
			tft_write_byte((T0_FPS>>8)&0X00FF);//Set FPS
			tft_write_byte(T0_FPS&0X00FF);

			tft_set_pointer(SSD1963_set_gpio_value);
			tft_write_byte(0x000F);//GPIO[3:0] out 1

			tft_set_pointer(SSD1963_set_gpio_conf);
			tft_write_byte(0x0007);//GPIO3=input, GPIO[2:0]=output
			tft_write_byte(0x0001);//GPIO0 normal

			tft_set_pointer(SSD1963_set_address_mode);//rotation
			if (rotate) tft_write_byte(0x0003);
			else tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_pixel_data_interface);//pixel data interface
			tft_write_byte(0x0003);

#ifdef LCD_DEBUG
	tft_set_pointer(SSD1963_get_pixel_data_interface); //pixel data interface
	printf_P(PSTR("\nint: %2.2x\n"), tft_read_byte());
#endif
			NutDelay(5);

			tft_set_pointer(SSD1963_set_display_on); //display on

			tft_set_pointer(SSD1963_set_dbc_conf);
			tft_write_byte(0x000d);

			break;

		case SSD1963_TYPE_1:
			#define		T1_HDP		479	//-->480
			#define		T1_HT		531
			#define		T1_HPS		43
			#define 	T1_LPS		8
			#define 	T1_HPW		1

			#define		T1_VDP		271	//-->272
			#define 	T1_VT		288
			#define		T1_VPS		12
			#define		T1_FPS		4
			#define		T1_VPW		10

			// Return used resolution
			current_lcd_type = SSD1963_TYPE_1;	// Set type for touch handling
			*lcd_res_x = T1_HDP;
			*lcd_res_y = T1_VDP;

			// enable wait
			XMCRA |= (1 << SRW00) | (1 << SRW01); // wait

			tft_set_pointer(SSD1963_set_pll_mn); //PLL multiplier, set PLL clock to 120M
			tft_write_byte(0x0023); //N=0x36 for 6.5M, 0x23 for 10M crystal
			tft_write_byte(0x0002);
			tft_write_byte(0x0004);
			tft_set_pointer(SSD1963_set_pll); // PLL enable
			tft_write_byte(0x0001);
			NutDelay(1);
			tft_set_pointer(SSD1963_set_pll);
			tft_write_byte(0x0003);
			NutDelay(5);
			tft_set_pointer(SSD1963_soft_reset); // software reset
			NutDelay(5);

			// disable wait
			XMCRA &= ~((1 << SRW00) | (1 << SRW01)); // wait

			tft_set_pointer(SSD1963_set_lshift_freq); //PLL setting for PCLK, depends on resolution
			tft_write_byte(0x0003);
			tft_write_byte(0x0033);
			tft_write_byte(0x0032);

			tft_set_pointer(SSD1963_set_lcd_mode); //LCD SPECIFICATION
			tft_write_byte(0x0000);
			tft_write_byte(0x0000);
			tft_write_byte((T1_HDP >> 8) & 0X00FF); //Set HDP
			tft_write_byte(T1_HDP & 0X00FF);
			tft_write_byte((T1_VDP >> 8) & 0X00FF); //Set VDP
			tft_write_byte(T1_VDP & 0X00FF);
			tft_write_byte(0x002D);

			tft_set_pointer(SSD1963_set_hori_period); //HSYNC
			tft_write_byte((T1_HT >> 8) & 0X00FF); //Set HT
			tft_write_byte(T1_HT & 0X00FF);
			tft_write_byte((T1_HPS >> 8) & 0X00FF); //Set HPS
			tft_write_byte(T1_HPS & 0X00FF);
			tft_write_byte(T1_HPW); //Set HPW
			tft_write_byte((T1_LPS >> 8) & 0X00FF); //Set HPS
			tft_write_byte(T1_LPS & 0X00FF);
			tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_vert_period); //VSYNC
			tft_write_byte((T1_VT >> 8) & 0X00FF); //Set VT
			tft_write_byte(T1_VT & 0X00FF);
			tft_write_byte((T1_VPS >> 8) & 0X00FF); //Set VPS
			tft_write_byte(T1_VPS & 0X00FF);
			tft_write_byte(T1_VPW); //Set VPW
			tft_write_byte((T1_FPS >> 8) & 0X00FF); //Set FPS
			tft_write_byte(T1_FPS & 0X00FF);

			tft_set_pointer(SSD1963_set_gpio_value);
			tft_write_byte(0x000F); //GPIO[3:0] out 1

			tft_set_pointer(SSD1963_set_gpio_conf);
			tft_write_byte(0x0007); //GPIO3=input, GPIO[2:0]=output
			tft_write_byte(0x0001); //GPIO0 normal

			tft_set_pointer(SSD1963_set_address_mode); //rotation
			if (rotate) tft_write_byte(0x0003);
			else tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_pixel_data_interface); //pixel data interface
			tft_write_byte(0x0003);

			NutDelay(5);

			tft_set_pointer(SSD1963_set_display_on); //display on

			tft_set_pointer(SSD1963_set_dbc_conf);
			tft_write_byte(0x000d);

			break;

		case SSD1963_TYPE_2:
			#define		T2_HDP		479	//-->480
			#define		T2_HT		531
			#define		T2_HPS		43
			#define 	T2_LPS		8
			#define 	T2_HPW		10

			#define		T2_VDP		271	//-->272
			#define 	T2_VT		288
			#define		T2_VPS		12
			#define		T2_FPS		4
			#define		T2_VPW		10

			// Return used resolution
			current_lcd_type = SSD1963_TYPE_2;	// Set type for touch handling
			*lcd_res_x = T2_HDP;
			*lcd_res_y = T2_VDP;

			// enable wait
			XMCRA |= (1 << SRW00) | (1 << SRW01); // wait

			tft_set_pointer(SSD1963_set_pll_mn); //PLL multiplier, set PLL clock to 120M
			tft_write_byte(0x002d); //N=0x36 for 6.5M, 0x23 for 10M crystal
			tft_write_byte(0x0002);
			tft_write_byte(0x0004);
			tft_set_pointer(SSD1963_set_pll); // PLL enable
			tft_write_byte(0x0001);
			NutDelay(1);
			tft_set_pointer(SSD1963_set_pll);
			tft_write_byte(0x0003);
			NutDelay(5);
			tft_set_pointer(SSD1963_soft_reset); // software reset
			NutDelay(5);

			// disable wait
			XMCRA &= ~((1 << SRW00) | (1 << SRW01)); // wait

			tft_set_pointer(SSD1963_set_lshift_freq); //PLL setting for PCLK, depends on resolution
			tft_write_byte(0x0000);
			tft_write_byte(0x00ff);
			tft_write_byte(0x00be);

			tft_set_pointer(SSD1963_set_lcd_mode); //LCD SPECIFICATION
			tft_write_byte(0x0020);
			tft_write_byte(0x0000);
			tft_write_byte((T2_HDP >> 8) & 0X00FF); //Set HDP
			tft_write_byte(T2_HDP & 0X00FF);
			tft_write_byte((T2_VDP >> 8) & 0X00FF); //Set VDP
			tft_write_byte(T2_VDP & 0X00FF);
			tft_write_byte(0x0000);
			NutDelay(5);

			tft_set_pointer(SSD1963_set_hori_period); //HSYNC
			tft_write_byte((T2_HT >> 8) & 0X00FF); //Set HT
			tft_write_byte(T2_HT & 0X00FF);
			tft_write_byte((T2_HPS >> 8) & 0X00FF); //Set HPS
			tft_write_byte(T2_HPS & 0X00FF);
			tft_write_byte(T2_HPW); //Set HPW
			tft_write_byte((T2_LPS >> 8) & 0X00FF); //Set HPS
			tft_write_byte(T2_LPS & 0X00FF);
			tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_vert_period); //VSYNC
			tft_write_byte((T2_VT >> 8) & 0X00FF); //Set VT
			tft_write_byte(T2_VT & 0X00FF);
			tft_write_byte((T2_VPS >> 8) & 0X00FF); //Set VPS
			tft_write_byte(T2_VPS & 0X00FF);
			tft_write_byte(T2_VPW); //Set VPW
			tft_write_byte((T2_FPS >> 8) & 0X00FF); //Set FPS
			tft_write_byte(T2_FPS & 0X00FF);

			tft_set_pointer(SSD1963_set_gpio_value);
			tft_write_byte(0x0000); //GPIO[3:0] out 1

			tft_set_pointer(SSD1963_set_gpio_conf);
			tft_write_byte(0x0000); //GPIO3=input, GPIO[2:0]=output
			tft_write_byte(0x0001); //GPIO0 normal

			tft_set_pointer(SSD1963_set_address_mode); //rotation
			if (rotate) tft_write_byte(0x0003);
			else tft_write_byte(0x0000);

			tft_set_pointer(SSD1963_set_pixel_data_interface); //pixel data interface
			tft_write_byte(0x0003);

			NutDelay(5);

			tft_set_pointer(SSD1963_set_display_on); //display on

			tft_set_pointer(SSD1963_set_dbc_conf);
			tft_write_byte(0x000d);

			break;

		default:	// Now we have a problem :-(
			*lcd_res_x = 0;
			*lcd_res_y = 0;
			break;
	}
}


// Handle touch event
void ssd1963_touch_pressed(void) {

	switch (current_lcd_type) {
		case SSD1963_TYPE_0:
			ly= (TP_X - SSD1963_50_Y_OFFSET) / SSD1963_50_Y_OFFSET_FACT;
			if (invert_touch_y)
				ly = get_max_y() - ly;
			if (ly < 0)
				ly = 0;
			if (ly > get_max_y())
				ly = get_max_y();

			lx= (TP_Y - SSD1963_50_X_OFFSET) / SSD1963_50_X_OFFSET_FACT;
			if (!invert_touch_x)	// needs to be inversed somehow?? Shouldn't it be like type 2
				lx = get_max_x() - lx;
			if (lx < 0)
				lx = 0;
			if (lx > get_max_x())
				lx = get_max_x();
			break;

		case SSD1963_TYPE_1:
			ly = (TP_Y - SSD1963_43_Y_OFFSET) / SSD1963_43_Y_OFFSET_FACT;
				if (invert_touch_y)
			ly = get_max_y() - ly;
				if (ly < 0)
			ly = 0;
				if (ly > get_max_y())
			ly = get_max_y();

			lx = (TP_X - SSD1963_43_X_OFFSET) / SSD1963_43_X_OFFSET_FACT;
			if (invert_touch_x)
				lx = get_max_x() - lx;
			if (lx < 0)
				lx = 0;
			if (lx > get_max_x())
				lx = get_max_x();
			break;

		case SSD1963_TYPE_2:
			ly = (TP_X - SSD1963_43_Y_OFFSET) / SSD1963_43_Y_OFFSET_FACT;
			if (invert_touch_y)
				ly = get_max_y() - ly;
			if (ly < 0)
				ly = 0;
			if (ly > get_max_y())
				ly = get_max_y();

			lx = (TP_Y - SSD1963_43_X_OFFSET) / SSD1963_43_X_OFFSET_FACT;
			if (invert_touch_x)
				lx = get_max_x() - lx;
			if (lx < 0)
				lx = 0;
			if (lx > get_max_x())
				lx = get_max_x();
			break;

		default:
		break;
	}
}




//
	//case SSD1963_TYPE_0:	// here we treat the 5" display using SSD1963
	//ly= (TP_X - SSD1963_50_Y_OFFSET) / SSD1963_50_Y_OFFSET_FACT;
	//if (!invert_touch_y)
	//ly = get_max_y() - ly;
	//if (ly < 0)
	//ly = 0;
	//if (ly > get_max_y())
	//ly = get_max_y();
//
	//lx= (SSD1963_50_X_OFFSET - TP_Y) / SSD1963_50_X_OFFSET_FACT;
	//#ifdef TFT_480_800_CTRL_SSD1963_INVERT_TOUCH_X
	//if (!invert_touch_x) {	// for the new 5.0" modules sold since b/o 2013
		//#else
		//if (invert_touch_x) { // for the 5.0" modules sold until b/o 2013
			//#endif //TFT_480_800_CTRL_SSD1963_INVERT_TOUCH_X
			//lx= get_max_x() - lx;
		//}
		//if (lx < 0)
		//lx = 0;
		//break;