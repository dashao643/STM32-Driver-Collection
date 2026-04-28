#ifndef __MATRIXKEY_H
#define __MATRIXKEY_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// 选择按键是上拉还是下拉

#define MATRIX_KEY_PULL_UP
// #define MATRIX_KEY_PULL_DOWN

#ifdef MATRIX_KEY_PULL_UP
#define MATRIX_KEY_NOT_PRESS       GPIO_PIN_SET
#define MATRIX_KEY_PRESSED         GPIO_PIN_RESET

#elifdef MATRIX_KEY_PULL_DOWN
#define MATRIX_KEY_NOT_PRESS       GPIO_PIN_RESET
#define MATRIX_KEY_PRESSED         GPIO_PIN_SET
#endif

#define MATRIX_KEY_INTERVAL_MS     20
#define MATRIX_KEY_ORDER           3
#define MATRIX_KEY_CNT             (MATRIX_KEY_ORDER * MATRIX_KEY_ORDER)

// typedef enum { 
//   MATRIX_KEY_NONE = 0,
//   MATRIX_KEY_UP1, 
//   MATRIX_KEY_UP2, 
//   MATRIX_KEY_UP3, 
//   MATRIX_KEY_NUM_MAX
// } MatrixKey_Num_e;
// 存储每个按键对应的值


typedef struct {
  GPIO_PinState rowPreState[MATRIX_KEY_CNT];
  GPIO_PinState rowCurState[MATRIX_KEY_CNT];
  GPIO_PinState colPreState[MATRIX_KEY_CNT];
  GPIO_PinState colCurState[MATRIX_KEY_CNT];
  bool keyReadFlag;
  uint32_t keyScanMs;
  uint8_t keyValue;
} MatrixKey_t;

void MatrixKey_Init(void);
uint8_t MatrixKey_Read(void);

#endif
