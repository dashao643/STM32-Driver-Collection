#ifndef __ST7735_H__
#define __ST7735_H__

// 160 * 128
// RGB:5 6 5 = 16

#define ST7735_INSTANCE                 SPI1
#define ST7735_HANDLE                   &hspi1
#define ST7735_TX_TIMEOUT_MS            100

#define ST7735_ROW                      160
#define ST7735_COL                      128

// RBG: 5 6 5
#define ST7735_BLACK                    0x0000
#define ST7735_GREY                     0x18E3
#define ST7735_WHITE                    0xFFFF
#define ST7735_RED                      0xF800
#define ST7735_GREEN                    0x07E0
#define ST7735_BLUE                     0x001F
#define ST7735_YELLOW                   0xFFE0

void ST7735_Init(void);
void ST7735_Clear(void);
void ST7735_Test(void);

#endif
