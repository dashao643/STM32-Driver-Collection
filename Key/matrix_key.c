#include "matrix_key.h"
#include "main.h"
#include "stm32f1xx_hal.h"

static GPIO_TypeDef *rowPorts[MATRIX_KEY_ORDER] = {
  KEY_ROW1_GPIO_Port,
  KEY_ROW2_GPIO_Port,
};
static const uint16_t rowPins[MATRIX_KEY_ORDER] = {
  KEY_ROW1_Pin,
  KEY_ROW2_Pin,
};

static GPIO_TypeDef *colPorts[MATRIX_KEY_ORDER] = {
  KEY_COL1_GPIO_Port,
  KEY_COL2_GPIO_Port,
};
static const uint16_t colPins[MATRIX_KEY_ORDER] = {
  KEY_COL1_Pin,
  KEY_COL2_Pin,
};

static MatrixKey_t matrixKey = {0};

static void setAllRowsHigh(void);
static void setSingleRowLow(GPIO_TypeDef *GPIOx, uint16_t pin);
static uint16_t keyScan(void);

static void setAllRowsHigh(void)
{
  for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++)
    HAL_GPIO_WritePin(rowPorts[i], rowPins[i], GPIO_PIN_SET);
}

static void setSingleRowLow(GPIO_TypeDef *GPIOx, uint16_t pin)
{
  HAL_GPIO_WritePin(GPIOx, pin, GPIO_PIN_RESET);
}

static uint16_t keyScan(void)
{
  uint16_t keyMask = 0;
  for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++){
    setSingleRowLow(rowPorts[i], rowPins[i]);

    for (uint8_t j = 0; j < MATRIX_KEY_ORDER; j++){
      if (HAL_GPIO_ReadPin(colPorts[j], colPins[j]) == MATRIX_KEY_PRESSED){
        uint8_t index = i * MATRIX_KEY_ORDER + j;
        keyMask |= (1 << index);
      }
    }
    setAllRowsHigh();
  }
  return keyMask;
}

/*-----------------------------------------------------------------*/

void MatrixKey_Init(void)
{
  matrixKey.preKey = KEY_NONE;
  matrixKey.curKey = KEY_NONE;
  matrixKey.scanTimer = HAL_GetTick();
  setAllRowsHigh();
}

uint16_t MatrixKey_Read(void)
{
  if (HAL_GetTick() - matrixKey.scanTimer < KEY_INTERVAL_MS)
    return 0;
    
  matrixKey.scanTimer = HAL_GetTick();
  
  uint16_t keyMask = 0;

  matrixKey.preKey = matrixKey.curKey;
  matrixKey.curKey = keyScan();

#ifdef KEY_MODE_TRIGGER
  if (matrixKey.preKey == KEY_NONE && matrixKey.curKey != KEY_NONE) {
    keyMask = matrixKey.curKey;
  }

#elif defined(KEY_MODE_RELEASE)
  if (matrixKey.preKey != KEY_NONE && matrixKey.curKey == KEY_NONE) {
    keyMask = matrixKey.preKey;
  }

#elif defined(KEY_MODE_HOLD)
  return matrixKey.curKey;
#endif
  return keyMask;
}
