#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "main.h"
#include <stdint.h>

// 选择按键是上拉还是下拉
#define PULL_UP 1
#define PULL_DOWN 0

#define KEY_MODE PULL_UP

#if KEY_MODE == PULL_UP
#define KEY_NOT_PRESS GPIO_PIN_SET
#define KEY_PRESSED   GPIO_PIN_RESET

#elif KEY_MODE == PULL_DOWN  // 显式判断下拉，而非else兜底
#define KEY_NOT_PRESS GPIO_PIN_RESET
#define KEY_PRESSED   GPIO_PIN_SET

#else
// 无效配置时，编译阶段直接报错，提示问题
#error "KEY_MODE配置错误!仅支持PULL_UP(1)或PULL_DOWN(0)!"
#endif

#define KEY_CNT 3

typedef enum { 
  KEY_NONE = 0,
  KEY_UP1, 
  KEY_UP2, 
  KEY_UP3, 
//   KEY_DOWN1, 
//   KEY_DOWN2,
//   KEY_DOWN3
} KeyNum;

extern uint8_t keyReadFlag;

void Key_Init();
KeyNum keyRead();

#endif
