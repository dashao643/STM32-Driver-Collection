#ifndef __MATRIXKEY_H
#define __MATRIXKEY_H

#include "key.h"

// 按键固定配置为上拉

#define MATRIX_KEY_RELEASE         GPIO_PIN_SET       // 按键释放
#define MATRIX_KEY_PRESSED         GPIO_PIN_RESET     // 按键按下

#define MATRIX_KEY_ORDER           3                  // 按键矩阵阶数
#define MATRIX_KEY_CNT             (MATRIX_KEY_ORDER * MATRIX_KEY_ORDER)

typedef enum {
  KEY_1 = 1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
} MatrixKey_e;

void MatrixKey_Init(void);
MatrixKey_e MatrixKey_Read(void);

#endif
