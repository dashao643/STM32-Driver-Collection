#ifndef __OLED_H__
#define __OLED_H__

#include "i2c.h"

#define ASCII_OFFSET 0x20
#define ASCII_LENGTH 95

void OLED_Init();
void OLED_Clear();
void OLED_ShowALL();
void OLED_SetReverse();
// 1<=x<=4 1<=y<=16
void OLED_Show_Char(uint8_t x, uint8_t y, char ch);
void OLED_Show_String(uint8_t x, uint8_t y, char str[]);
void OLED_Show_DecNumber(uint8_t x, uint8_t y, int32_t number, uint8_t maxLen);

#endif
