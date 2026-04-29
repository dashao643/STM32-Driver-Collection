#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

// 选择按键是上拉还是下拉
#define KEY_PULL_UP
// #define PULL_DOWN

#ifdef KEY_PULL_UP
#define KEY_NOT_PRESS GPIO_PIN_SET
#define KEY_PRESSED   GPIO_PIN_RESET

#elifdef KEY_PULL_DOWN
#define KEY_NOT_PRESS GPIO_PIN_RESET
#define KEY_PRESSED   GPIO_PIN_SET
#endif

#define KEY_INTERVAL_MS            20
#define KEY_CNT                    3

typedef enum { 
  KEY_NONE = 0,
  KEY_UP1, 
  KEY_UP2, 
  KEY_UP3, 
//   KEY_DOWN1, 
//   KEY_DOWN2,
//   KEY_DOWN3
} KeyNum_e;

typedef struct {
  uint8_t preKey;
  uint8_t curKey;
  uint32_t keyScanMs;
  uint8_t keyValue;
} Key_t;

void Key_Init(void);
KeyNum_e key_Read(void);

#endif
