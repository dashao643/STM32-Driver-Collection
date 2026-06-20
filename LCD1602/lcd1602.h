#ifndef __LCD1602_H__
#define __LCD1602_H__

#include <stdint.h>
#include <stdbool.h>

#define LCD1602_WAIT_TIMEOUT_MS       20    // 等待忙标志超时时间

#define LCD1602_CLEAR_DISPLAY         0x01  // 清屏,地址AC=0,光标归位,不改变移位 1
#define LCD1602_RETURN_HOME           0x02  // 地址AC=0,光标归位 0
#define LCD1602_ENTRY_MODE_ADD        0x06  // AC和光标自增,显示不移位 1
#define LCD1602_DISPLAY_ON            0x0C  // 显示开,光标不显示,不闪烁 1
#define LCD1602_DISPLAY_SHIFT         0x1C  // 光标和显示向右移位 0
#define LCD1602_FUNCTION_SET          0x38  // 8位数据总线,2行显示,5*7点阵 1
#define LCD1602_SET_CGRAM_ADDRESS     0x40  // 用户自定义字符地址送入AC 0
#define LCD1602_SET_DDRAM_ADDRESS     0x80  // 内置字符地址送入AC(第一行0x00-0x27,第二行0x40-0x67) 0
// #define LCD1602_READ_BUSY_FLAG
// #define LCD1602_WRITE_DATA_TO_DDRAM   

void LCD1602_Init(void);
void LCD1602_Test(void);

#endif
