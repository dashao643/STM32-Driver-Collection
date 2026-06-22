#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>

// 按键固定配置为上拉
// 只有HOLD模式支持多按键同时读取

/********************* ↓选择按键触发模式↓ *******************/
// #define KEY_MODE_TRIGGER        // 按下瞬间有效
// #define KEY_MODE_RELEASE        // 松开瞬间有效
#define KEY_MODE_HOLD           // 按住不放有效
/********************* ↑选择按键触发模式↑ *******************/

#define KEY_UNPRESSED       GPIO_PIN_SET       // 按键空闲
#define KEY_PRESSED         GPIO_PIN_RESET     // 按键按下

#define KEY_INTERVAL_MS            20
#define KEY_CNT                    4

typedef enum { 
  KEY_NONE = 0,
  KEY_1 = 0x01,
  KEY_2 = 0x02,
  KEY_3 = 0x04,
  KEY_4 = 0x08,
//   KEY_DOWN1, 
//   KEY_DOWN2,
//   KEY_DOWN3
} Key_e;

typedef struct {
  uint16_t preKey;
  uint16_t curKey;
  uint32_t scanTimer;
} Key_t;

void Key_Init(void);
uint16_t Key_Read(void);

#endif
