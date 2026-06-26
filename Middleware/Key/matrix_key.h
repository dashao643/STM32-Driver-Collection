#ifndef __MATRIX_KEY_H
#define __MATRIX_KEY_H

#include <stdint.h>
#include <stdbool.h>

// 按键固定配置为上拉
// 行输出,列输入

/********************* ↓选择按键触发模式↓ *******************/
// #define KEY_MODE_TRIGGER        // 按下瞬间有效
#define KEY_MODE_RELEASE        // 松开瞬间有效
// #define KEY_MODE_HOLD           // 按住不放有效(矩阵键盘最多支持同时触发两个按键)
/********************* ↑选择按键触发模式↑ *******************/

#define MATRIX_KEY_UNPRESSED       GPIO_PIN_SET       // 按键空闲
#define MATRIX_KEY_PRESSED         GPIO_PIN_RESET     // 按键按下

#define MATRIX_KEY_ORDER           4                  // 按键矩阵阶数
#define MATRIX_KEY_CNT             (MATRIX_KEY_ORDER * MATRIX_KEY_ORDER)

#define KEY_INTERVAL_MS            20

typedef enum {
  KEY_NONE = 0,
  KEY_1 = 0x0001,
  KEY_2 = 0x0002,
  KEY_3 = 0x0004,
  KEY_4 = 0x0008,
  KEY_5 = 0x0010,
  KEY_6 = 0x0020,
  KEY_7 = 0x0040,
  KEY_8 = 0x0080,
  KEY_9 = 0x0100,
  KEY_10 = 0x0200,
  KEY_11 = 0x0400,
  KEY_12 = 0x0800,
  KEY_13 = 0x1000,
  KEY_14 = 0x2000,
  KEY_15 = 0x4000,
  KEY_16 = 0x8000,
} MatrixKey_e;

typedef struct {
  uint16_t preKey;
  uint16_t curKey;
  uint32_t scanTimer;
} MatrixKey_t;

void MatrixKey_Init(void);
uint16_t MatrixKey_Read(void);

#endif
