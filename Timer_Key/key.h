#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "main.h"
#include <stdint.h>

// 选择按键是上拉还是下拉
// #define PULL_UP_KEY
#define PULL_DOWN_KEY

#ifdef PULL_UP_KEY
#define KEY_NOT_PRESS GPIO_PIN_SET
#define KEY_PRESSED   GPIO_PIN_RESET
#endif

#ifdef PULL_DOWN_KEY
#define KEY_NOT_PRESS GPIO_PIN_RESET
#define KEY_PRESSED   GPIO_PIN_SET
#endif

#define KEY_CNT 2

typedef enum { 
  KEY_NONE = 0,
  KEY_UP1, 
  KEY_UP2, 
//   KEY_DOWN1, 
//   KEY_DOWN2,
//   KEY_DOWN3
} KeyNum;

extern uint8_t keyReadFlag;

void Key_Init();
KeyNum keyRead();

#endif
