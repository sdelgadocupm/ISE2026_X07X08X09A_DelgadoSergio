#include "stm32f4xx_hal.h"

#ifndef __lcd_H
#define __lcd_H
	int Init_lcd (void);  
	void LCD_reset(void);
	void delay(uint32_t n_microsegundos);
	void LCD_wr_data(unsigned char data);
	void LCD_wr_cmd(unsigned char cmd);
	void LCD_Init(void);
	void LCD_update(void);
	void symbolToLocalBuffer_L1(uint8_t symbol);
	void symbolToLocalBuffer_L2(uint8_t symbol);
	void printLCD(char text[], uint8_t line);
	void LCD_clean1(void);
	void LCD_clean2(void);
	void LCD_clean(void);
#endif
