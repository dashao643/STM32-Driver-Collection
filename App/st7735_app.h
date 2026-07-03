#ifndef __ST7735_APP_H__
#define __ST7735_APP_H__

#include "st7735.h"

#define ST7735_APP_INTERVAL_MS        33

void ST7735_APP_PlayTask(const uint8_t (*images)[ST7735_IMAGE_128_X_128], uint8_t cnt);

#endif
