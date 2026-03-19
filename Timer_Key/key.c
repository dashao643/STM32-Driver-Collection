#include "key.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

uint8_t keyReadFlag = 0;

// 定义按键引脚表，匹配循环中的组数
static GPIO_TypeDef *key_ports[KEY_CNT] = {KEY_UP1_GPIO_Port,KEY_UP2_GPIO_Port,KEY_UP3_GPIO_Port};
static const uint16_t key_pins[KEY_CNT] = {KEY_UP1_Pin,KEY_UP2_Pin,KEY_UP3_Pin};

// static GPIO_TypeDef *key_ports[KEY_CNT] = {KEY_DOWN1_GPIO_Port,KEY_DOWN2_GPIO_Port,KEY_DOWN3_GPIO_Port};
// static const uint16_t key_pins[KEY_CNT] = {KEY_DOWN1_Pin,KEY_DOWN2_Pin,KEY_DOWN3_Pin};

static GPIO_PinState preState[KEY_CNT];
static GPIO_PinState curState[KEY_CNT];

//初始化静态索引数组
void Key_Init(){
  for (uint8_t i = 0; i < KEY_CNT; i++) {
    preState[i] = KEY_NOT_PRESS;
    curState[i] = KEY_NOT_PRESS;
  }
}

KeyNum keyRead() {
  KeyNum res = KEY_NONE;

  if (keyReadFlag == 1) {
    keyReadFlag = 0;

    for (uint8_t i = 0; i < KEY_CNT; i++) {
      preState[i] = curState[i];
      curState[i] = HAL_GPIO_ReadPin(key_ports[i], key_pins[i]);
      if ((preState[i] == KEY_PRESSED) && (curState[i] == KEY_NOT_PRESS))   //上次按下，这次松开
        res = i + 1;                                                        //i + 1 跳过KEY_NONE的0
    }
  }

  return res;
}