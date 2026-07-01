#include "stm32f4xx_hal.h"
#include "st7735_app.h"

#include <stdint.h>

static uint32_t timer = 0;
static uint8_t imageIdx = 0;

/**
 * @brief 支持 128 X 128 动画大小
 * 
 * @param images 图片二维数组,每张图片像素为 128 X 128
 * @param cnt 动画张数
 */
void ST7735_APP_PlayTask(const uint8_t (*images)[ST7735_IMAGE_128_X_128], uint8_t cnt)
{
  if(HAL_GetTick() - timer < ST7735_APP_INTERVAL_MS)
    return;

  timer = HAL_GetTick();

  ST7735_ShowImage(images[imageIdx], ST7735_IMAGE_128_X_128);
  imageIdx++;

  if(imageIdx == cnt)
    imageIdx = 0;
}