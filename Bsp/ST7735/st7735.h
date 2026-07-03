#ifndef __ST7735_H__
#define __ST7735_H__

// 160 * 128
// RGB: 5 6 5 = 16

#include <stdint.h>

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

#define ST7735_IMAGE_160_X_128          0       // 图片尺寸: 长160,宽128(竖屏显示)
#define ST7735_IMAGE_128_X_128          1       // 图片尺寸: 长128,宽128(根据默认方向)
#define ST7735_IMAGE_128_X_160          2       // 图片尺寸: 长160,宽128(横屏显示)

void ST7735_Init(void);
void ST7735_Clear(void);
void ST7735_ShowChar(uint8_t row, uint8_t col, char ch, uint16_t color);
void ST7735_ShowString(uint8_t row, uint8_t col, const char *str, uint16_t color);
void ST7735_ShowImage(const uint8_t *image, uint8_t imageWindow);

#endif
