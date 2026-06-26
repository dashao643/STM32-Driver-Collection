#include "stm32f4xx_hal.h"
#include "matrix_key.h"
#include "general.h"
#include "main.h"

// 根据阶数添加
static GPIO_PortPin_t rowPortPin[MATRIX_KEY_ORDER] = {
  {KEY_ROW1_GPIO_Port, KEY_ROW1_Pin},
  {KEY_ROW2_GPIO_Port, KEY_ROW2_Pin},
  {KEY_ROW3_GPIO_Port, KEY_ROW3_Pin},
  {KEY_ROW4_GPIO_Port, KEY_ROW4_Pin}
};

static GPIO_PortPin_t colPortPin[MATRIX_KEY_ORDER] = {
  {KEY_COL1_GPIO_Port, KEY_COL1_Pin},
  {KEY_COL2_GPIO_Port, KEY_COL2_Pin},
  {KEY_COL3_GPIO_Port, KEY_COL3_Pin},
  {KEY_COL4_GPIO_Port, KEY_COL4_Pin}
};

// static GPIO_TypeDef *rowPorts[MATRIX_KEY_ORDER] = {
//   KEY_ROW1_GPIO_Port,
//   KEY_ROW2_GPIO_Port,
//   KEY_ROW3_GPIO_Port
// };
// static const uint16_t rowPins[MATRIX_KEY_ORDER] = {
//   KEY_ROW1_Pin,
//   KEY_ROW2_Pin,
//   KEY_ROW3_Pin
// };

// static GPIO_TypeDef *colPorts[MATRIX_KEY_ORDER] = {
//   KEY_COL1_GPIO_Port,
//   KEY_COL2_GPIO_Port,
//   KEY_COL3_GPIO_Port
// };
// static const uint16_t colPins[MATRIX_KEY_ORDER] = {
//   KEY_COL1_Pin,
//   KEY_COL2_Pin,
//   KEY_COL3_Pin
// };

static MatrixKey_t matrixKey = {0};

static inline void setAllRowsHigh(void);
static inline void setSingleRowLow(GPIO_TypeDef *GPIOx, uint16_t pin);
static uint16_t keyScan(void);

static inline void setAllRowsHigh(void)
{
  for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++)
    // HAL_GPIO_WritePin(rowPorts[i], rowPins[i], GPIO_PIN_SET);
    HAL_GPIO_WritePin(rowPortPin[i].port, rowPortPin[i].pin, GPIO_PIN_SET);
}

static inline void setSingleRowLow(GPIO_TypeDef *GPIOx, uint16_t pin)
{
  HAL_GPIO_WritePin(GPIOx, pin, GPIO_PIN_RESET);
}

static uint16_t keyScan(void)
{
  uint16_t keyMask = 0;
  for (uint8_t i = 0; i < MATRIX_KEY_ORDER; i++){
    setSingleRowLow(rowPortPin[i].port, rowPortPin[i].pin);

    for (uint8_t j = 0; j < MATRIX_KEY_ORDER; j++){
      if (HAL_GPIO_ReadPin(colPortPin[j].port, colPortPin[j].pin) == MATRIX_KEY_PRESSED){
        uint8_t index = i * MATRIX_KEY_ORDER + j;
        keyMask |= (1 << index);
      }
    }
    setAllRowsHigh();
  }
  return keyMask;
}

uint16_t MatrixKey_Read(void)
{
  uint16_t keyMask = 0;
  if (HAL_GetTick() - matrixKey.scanTimer < KEY_INTERVAL_MS)
    return keyMask;
  matrixKey.scanTimer = HAL_GetTick();

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

void MatrixKey_Init(void)
{
  matrixKey.preKey = KEY_NONE;
  matrixKey.curKey = KEY_NONE;
  matrixKey.scanTimer = HAL_GetTick();
  setAllRowsHigh();
}
