#ifndef __OLED_H__
#define __OLED_H__

#include <stdint.h>

// SCL 推挽 
// SDA 开漏

// I2C选择
// #define I2C_SOFTWARE           // 软件I2C
#define I2C_HARDWARE              // 硬件I2C

#ifdef I2C_HARDWARE
#define OLED_HANDLE               &hi2c1
#define OLED_TIME_OUT             50
#endif

#define ASCII_OFFSET              0x20
#define ASCII_LENGTH              95

#define OLED_I2C_SLAVE_ADDR       0x78
#define OLED_I2C_CMD              0x00 // 写命令
#define OLED_I2C_DATA             0x40 // 写数据

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowALL(void);
void OLED_SetReverse(void);

// 4行，16列
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char str[]);
void OLED_ShowDecNumber(uint8_t x, uint8_t y, int32_t number, uint8_t numLen);
void OLED_ShowHexNumber(uint8_t x, uint8_t y, const uint8_t data[], uint8_t size);

void OLED_StrWriteLine(uint8_t row, const char str[]);
void OLED_ShowImage(const uint8_t *image);

#endif
