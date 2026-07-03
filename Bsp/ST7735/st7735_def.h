#ifndef __ST7735_DEF_H__
#define __ST7735_DEF_H__

#define ST7735_SOFTWARE_RESET               0x01  // 软件复位: 0 执行后等待120ms,加载默认值
#define ST7735_SLEEP_IN                     0x10  // Sleep In: 0
#define ST7735_SLEEP_OUT                    0x11  // Sleep Out: 0
#define ST7735_PARTIAL_DISPLAY_MODE_ON      0x12  // Partial Display Mode On: 0
#define ST7735_NORMAL_DISPLAY_MODE_ON       0x13  // Normal Display Mode On: 0
#define ST7735_DISPLAY_INVERSION_OFF        0x20  // Display Inversion Off: 0
#define ST7735_DISPLAY_INVERSION_ON         0x21  // Display Inversion On: 0
#define ST7735_GAMMA_SET                    0x26  // Gamma Set: 1
#define ST7735_DISPLAY_OFF                  0x28  // Display Off: 0
#define ST7735_DISPLAY_ON                   0x29  // Display On: 0
#define ST7735_COLUMN_ADDRESS_SET           0x2A  // Column Address Set: 4
#define ST7735_ROW_ADDRESS_SET              0x2B  // Row Address Set: 4
#define ST7735_MEMORY_WRITE                 0x2C  // Memory Write: n
#define ST7735_PARTIAL_AREA                 0x30  // Partial Area: 4
#define ST7735_SCROLL_AREA_SET              0x33  // Scroll Area Set: 6
#define ST7735_TEARING_EFFECT_LINE_OFF      0x34  // Tearing Effect Line OFF: 0
#define ST7735_TEARING_EFFECT_LINE_ON       0x35  // Tearing Effect Line ON: 0
#define ST7735_MEMORY_DATA_ACCESS_CONTROL   0x36  // Memory Data Access Control: 1 (Row/Column Exchange)
#define ST7735_VERTICAL_SCROLL_START        0x37  // Vertical Scroll Start Address of RAM: 2
#define ST7735_IDLE_MODE_OFF                0x38  // Idle Mode Off: 0
#define ST7735_IDLE_MODE_ON                 0x39  // Idle Mode On: 0
#define ST7735_INTERFACE_PIXEL_FORMAT       0x3A  // Interface Pixel Format: 1

#define ST7735_FRAME_RATE_CONTROL           0xB1  // Frame Rate Control (In normal mode/ Full colors): 3
#define ST7735_DISPLAY_INVERSION_CONTROL    0xB4  // Display Inversion Control: 1
#define ST7735_POWER_CONTROL_1              0xC0  // Power Control 1: 3
#define ST7735_POWER_CONTROL_2              0xC1  // Power Control 2: 1
#define ST7735_POWER_CONTROL_3              0xC2  // Power Control 3: 2
#define ST7735_VCOM_CONTROL_1               0xC5  // VCOM Control 1: 1

#define ST7735_GAMMA_POS_POLARITY           0xE0  // Gamma (+ polarity) Correction Characteristics Setting: 16
#define ST7735_GAMMA_NEG_POLARITY           0xE1  // Gamma (- polarity) Correction Characteristics Setting: 16

#endif
