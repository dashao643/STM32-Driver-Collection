#include "key.h"
#include "main.h"
#include "stm32f1xx_hal.h"

#include <stdbool.h>

// 定义按键引脚表，匹配循环中的组数
static GPIO_TypeDef *keyPorts[KEY_CNT] = {
  KEY_UP1_GPIO_Port,
  KEY_UP2_GPIO_Port,
  KEY_UP3_GPIO_Port
};
static const uint16_t keyPins[KEY_CNT] = {
  KEY_UP1_Pin,
  KEY_UP2_Pin,
  KEY_UP3_Pin
};

// static GPIO_TypeDef *key_ports[KEY_CNT] = {
//   KEY_DOWN1_GPIO_Port,
//   KEY_DOWN2_GPIO_Port,
//   KEY_DOWN3_GPIO_Port
// };
// static const uint16_t key_pins[KEY_CNT] = {
//   KEY_DOWN1_Pin,
//   KEY_DOWN2_Pin,
//   KEY_DOWN3_Pin
// };

static Key_t key = {0};

// 初始化静态索引数组
void Key_Init(void)
{
  key.preKey = KEY_NONE;
  key.curKey = KEY_NONE;
  key.keyValue = KEY_NONE;
  key.keyScanMs = HAL_GetTick();
}

static KeyNum_e keyScan(void)
{
  KeyNum_e key = KEY_NONE;

  for (uint8_t i = 0; i < KEY_CNT; i++) {
    if (HAL_GPIO_ReadPin(keyPorts[i], keyPins[i]) == KEY_PRESSED) {
      key = i + 1;
      break;
    }
  }

  return key;
}

KeyNum_e Key_Read(void) 
{
  key.keyValue = KEY_NONE;

  // 间隔 KEY_INTERVAL_MS 扫描一次
  if ((HAL_GetTick() - key.keyScanMs) >= KEY_INTERVAL_MS) {
    key.keyScanMs = HAL_GetTick();

    // 保存上一次状态
    key.preKey = key.curKey;
    // 读取当前按下的键
    key.curKey = keyScan();

    // 上一次有按键按下，当前松开
    if (key.preKey != KEY_NONE && key.curKey == KEY_NONE) {
      // 赋值为上次按键的键值（此时 curKey 已为 KEY_NONE ）
      key.keyValue = key.preKey;
    }
  }

  return key.keyValue;
}