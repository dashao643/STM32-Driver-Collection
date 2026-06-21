#ifndef __LCD1602_H__
#define __LCD1602_H__

#include <stdint.h>
#include <stdbool.h>

// VDD 电源接5V
// VO 接 GND

// #define LCD1602_WAIT_TIMEOUT_MS       20    // 等待忙标志超时时间

#define LCD1602_CLEAR_DISPLAY         0x01  // 清屏,地址AC=0,光标归位,不改变移位 1
#define LCD1602_RETURN_HOME           0x02  // 地址AC=0,光标归位 0
#define LCD1602_ENTRY_MODE_ADD        0x06  // AC和光标自增,显示不移位 1
#define LCD1602_DISPLAY_ON            0x0C  // 显示开,光标不显示,不闪烁 1
#define LCD1602_DISPLAY_SHIFT_L       0x18  // 光标和显示向左移位 0
#define LCD1602_DISPLAY_SHIFT_R       0x1C  // 光标和显示向右移位 0
#define LCD1602_FUNCTION_SET          0x38  // 8位数据总线,2行显示,5*7点阵 1
#define LCD1602_SET_CGRAM_ADDRESS     0x40  // 用户自定义字符地址送入AC 0
#define LCD1602_SET_DDRAM_ADDRESS     0x80  // 内置字符地址送入AC(第一行0x00-0x27,第二行0x40-0x67) 0

void LCD1602_Init(void);
void LCD1602_Clear(void);
void LCD1602_ShowAll(void);

void LCD1602_ShowString(uint8_t row, uint8_t col, const char *str);
void LCD1602_ShowDecNum(uint8_t row, uint8_t col, int32_t num, uint8_t numLen);
void LCD1602_ShowHexNum(uint8_t row, uint8_t col, const uint8_t *data, uint8_t size);

void LCD1602_WriteDDRAM(uint8_t row, const char *str);

void LCD1602_MoveLeft(void);
void LCD1602_MoveRight(void);

#endif
