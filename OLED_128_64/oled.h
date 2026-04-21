#ifndef __OLED_H__
#define __OLED_H__

#include "i2c.h"
#include <stdint.h>

#define ASCII_OFFSET  0x20
#define ASCII_LENGTH  95
#define TIME_OUT      50
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowALL(void);
void OLED_SetReverse(void);

// 4行，16列
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, char str[]);
void OLED_ShowDecNumber(uint8_t x, uint8_t y, int32_t number, uint8_t numLen);
void OLED_ShowHexNumber(uint8_t x, uint8_t y, uint8_t data[], uint8_t numLen);

#endif
