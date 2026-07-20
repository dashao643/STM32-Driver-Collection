#ifndef __ST7735_H__
#define __ST7735_H__

#include <stdint.h>

/*
SPI配置:
Transmit Only Master 模式: 只发送,不接收
MSB First
CPOL: Low
CPHA: 1 Edge

SCL -> SCK
SDA -> MOSI
CS  -> 推挽输出
RST -> 推挽输出
DC  -> 推挽输出
*/

#define ST7735_INSTANCE                 SPI1
#define ST7735_HANDLE                   &hspi1
#define ST7735_TX_TIMEOUT_MS            100

#define ST7735_ROW_PIXEL                160
#define ST7735_COL_PIXEL                128

#define ST7735_ROW_CNT                  (ST7735_ROW_PIXEL / CHARACTER_HEIGHT)  // 字符行数: 10
#define ST7735_COL_CNT                  (ST7735_COL_PIXEL / CHARACTER_WIDTH)   // 字符列数: 16

// RBG: 5 6 5
#define ST7735_WHITE                    0xFFFF
#define ST7735_BLACK                    0x0000
#define ST7735_GREY                     0x18E3
#define ST7735_RED                      0xF800
#define ST7735_GREEN                    0x07E0
#define ST7735_BLUE                     0x001F
#define ST7735_YELLOW                   0xFFE0

#define ST7735_IMAGE_128_X_160          40960       // 图片尺寸: 宽128,长160(原生竖屏)
#define ST7735_IMAGE_128_X_128          32768       // 图片尺寸: 宽128,长128(128*128*2)

void ST7735_Init(void);
void ST7735_Clear(void);

void ST7735_ShowChar(uint8_t row, uint8_t col, char ch, uint16_t color);
void ST7735_ShowString(uint8_t row, uint8_t col, const char *str, uint16_t color);
void ST7735_ShowDecNumber(uint8_t row, uint8_t col, int32_t number, uint8_t numLen);

void ST7735_ShowImage(const uint8_t *image, uint16_t imageWindow);
void ST7735_ImageStream(const uint8_t *image, uint8_t curTimes, uint8_t sumTimes, uint16_t imageWindow);
void ST7735_ImageStreamFix(const uint8_t *image, uint8_t curTimes, uint16_t imageWindow);

void ST7735_Scroll(void);

#endif
