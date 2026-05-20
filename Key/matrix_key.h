#ifndef __MATRIX_KEY_H
#define __MATRIX_KEY_H

#include <stdint.h>
#include <stdbool.h>

// 按键固定配置为上拉

/********************* ↓选择按键触发模式↓ *******************/
// #define KEY_MODE_TRIGGER        // 按下瞬间有效
// #define KEY_MODE_RELEASE        // 松开瞬间有效
#define KEY_MODE_HOLD           // 按住不放有效(矩阵键盘最多支持同时触发两个按键)
/********************* ↑选择按键触发模式↑ *******************/

#define MATRIX_KEY_UNPRESSED       GPIO_PIN_SET       // 按键空闲
#define MATRIX_KEY_PRESSED         GPIO_PIN_RESET     // 按键按下

#define MATRIX_KEY_ORDER           2                  // 按键矩阵阶数
#define MATRIX_KEY_CNT             (MATRIX_KEY_ORDER * MATRIX_KEY_ORDER)

#define KEY_INTERVAL_MS            20

typedef enum {
  KEY_NONE = 0,
  KEY_1 = 0x01,
  KEY_2 = 0x02,
  KEY_3 = 0x04,
  KEY_4 = 0x08,
  // KEY_5,
  // KEY_6,
  // KEY_7,
  // KEY_8,
  // KEY_9,
} MatrixKey_e;

typedef struct {
  uint16_t preKey;
  uint16_t curKey;
  uint32_t scanTimer;
} MatrixKey_t;

void MatrixKey_Init(void);
uint16_t MatrixKey_Read(void);

#endif
