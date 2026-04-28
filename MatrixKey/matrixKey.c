#include "matrixKey.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdbool.h>

const uint8_t matrixKeysValue[MATRIX_KEY_ORDER][MATRIX_KEY_ORDER] = {
	{1, 2, 3}, 
	{4, 5, 6}, 
	{7, 8, 9},
};

// 横排按键引脚表
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
// 竖排按键引脚表
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

static MatrixKey_t matrixKey = {0};

void MatrixKey_Init(void)
{
  matrixKey.keyReadFlag = false;
  matrixKey.keyScanMs = HAL_GetTick();
  matrixKey.keyValue = 0;
  for (uint8_t i = 0; i < MATRIX_KEY_CNT; i++) {
    matrixKey.rowPreState[i] = MATRIX_KEY_NOT_PRESS;
    matrixKey.rowCurState[i] = MATRIX_KEY_NOT_PRESS;
    matrixKey.colPreState[i] = MATRIX_KEY_NOT_PRESS;
    matrixKey.colCurState[i] = MATRIX_KEY_NOT_PRESS;
  }
}

uint8_t MatrixKey_Read(void) 
{
  matrixKey.keyValue = 0;
  bool colHaveSan = false;

  if ((HAL_GetTick() - matrixKey.keyScanMs) >= MATRIX_KEY_INTERVAL_MS) {
    matrixKey.keyScanMs = HAL_GetTick();
    matrixKey.keyReadFlag = true;
  }

  if (matrixKey.keyReadFlag) {
    matrixKey.keyReadFlag = false;

    // 扫描横排
    for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++) {
      matrixKey.rowPreState[i] = matrixKey.rowCurState[i];
      matrixKey.rowCurState[i] = HAL_GPIO_ReadPin(rowPorts[i], rowPins[i]);

      // 横排中有按键触发，再扫描竖排
      if ((matrixKey.rowPreState[i] == MATRIX_KEY_PRESSED) && (matrixKey.rowCurState[i] == MATRIX_KEY_NOT_PRESS)) {
        for (uint8_t j = 0; j < MATRIX_KEY_ORDER; j++) {
          matrixKey.colPreState[j] = matrixKey.colCurState[j];
          matrixKey.colCurState[j] = HAL_GPIO_ReadPin(colPorts[j], colPins[j]);

          if ((matrixKey.colPreState[j] == MATRIX_KEY_PRESSED) && (matrixKey.colCurState[j] == MATRIX_KEY_NOT_PRESS))   
            matrixKey.keyValue = matrixKeysValue[i][j];                                                
        }
        colHaveSan = true;
      }        
    }
    // 如果竖排没有扫描，扫描竖排
    if(!colHaveSan){
      for (uint8_t j = 0; j < MATRIX_KEY_ORDER; j++) {
        matrixKey.colPreState[j] = matrixKey.colCurState[j];
        matrixKey.colCurState[j] = HAL_GPIO_ReadPin(colPorts[j], colPins[j]);
      }
    }
  }

  return matrixKey.keyValue;
}