#include "matrixKey.h"
#include "main.h"
#include "stm32f1xx_hal.h"

const uint8_t matrixKeysValue[MATRIX_KEY_ORDER][MATRIX_KEY_ORDER] = {
	{KEY_1, KEY_2, KEY_3},
	{KEY_4, KEY_5, KEY_6},
	{KEY_7, KEY_8, KEY_9},
};

static GPIO_TypeDef *rowPorts[MATRIX_KEY_ORDER] = {
  KEY_ROW1_GPIO_Port,
  KEY_ROW2_GPIO_Port,
  KEY_ROW3_GPIO_Port
};
static const uint16_t rowPins[MATRIX_KEY_ORDER] = {
  KEY_ROW1_Pin,
  KEY_ROW2_Pin,
  KEY_ROW3_Pin,
};

static GPIO_TypeDef *colPorts[MATRIX_KEY_ORDER] = {
  KEY_COL1_GPIO_Port,
  KEY_COL2_GPIO_Port,
  KEY_COL3_GPIO_Port
};
static const uint16_t colPins[MATRIX_KEY_ORDER] = {
  KEY_COL1_Pin,
  KEY_COL2_Pin,
  KEY_COL3_Pin,
};

static Key_t matrixKey = {0};

static void setAllRowsHigh(void)
{
  for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++)
    HAL_GPIO_WritePin(rowPorts[i], rowPins[i], GPIO_PIN_SET);
}

static void setSingleRowLow(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}

// 扫描一轮，返回当前按下的按键值
static MatrixKey_e keyScan(void)
{
  uint8_t key = KEY_NONE;

  for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++) {
    // 循环拉高每一行
    setSingleRowLow(rowPorts[i], rowPins[i]);

    // 判断每一行对应列是否按下
    for (uint8_t j = 0; j < MATRIX_KEY_ORDER; j++) {

      if (HAL_GPIO_ReadPin(colPorts[j], colPins[j]) == MATRIX_KEY_PRESSED) {
        key = matrixKeysValue[i][j];
        break;
      }
    }
    setAllRowsHigh();

    if (key != KEY_NONE) 
      break;
  }

  return key;
}

void MatrixKey_Init(void)
{
  matrixKey.preKey = KEY_NONE;
  matrixKey.curKey = KEY_NONE;
  matrixKey.keyValue = KEY_NONE;
  matrixKey.keyScanMs = HAL_GetTick();
  setAllRowsHigh();
}

MatrixKey_e MatrixKey_Read(void)
{
  matrixKey.keyValue = KEY_NONE;

  // 间隔 MATRIX_KEY_INTERVAL_MS 扫描一次
  if ((HAL_GetTick() - matrixKey.keyScanMs) >= KEY_INTERVAL_MS) {
    matrixKey.keyScanMs = HAL_GetTick();

    // 保存上一次状态
    matrixKey.preKey = matrixKey.curKey;
    // 读取当前按下的键
    matrixKey.curKey = keyScan();

    // 上一次有按键按下，当前松开
    if (matrixKey.preKey != KEY_NONE && matrixKey.curKey == KEY_NONE) {
      // 赋值为上次按键的键值（此时 curKey 已为 KEY_NONE ）
      matrixKey.keyValue = matrixKey.preKey;
    }
  }

  return matrixKey.keyValue;
}