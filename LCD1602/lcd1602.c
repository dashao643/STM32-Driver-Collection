#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "lcd1602.h"
#include "general.h"

/*
RS=H，DB7-DB0发数据   RS=L，DB7-DB0发指令
R/W=H，E=H，数据被读到DB7-DB0
R/W=L，E=H->L,数据被写到指令寄存器(IR)或控制寄存器(DR)

显示数据缓冲区DDRAM
字符发生器CGROM,CGRAM
*/
static GPIO_PortPin_t dataBit[] = {
  {LCD1602_D0_GPIO_Port, LCD1602_D0_Pin},
  {LCD1602_D1_GPIO_Port, LCD1602_D1_Pin},
  {LCD1602_D2_GPIO_Port, LCD1602_D2_Pin},
  {LCD1602_D3_GPIO_Port, LCD1602_D3_Pin},
  {LCD1602_D4_GPIO_Port, LCD1602_D4_Pin},
  {LCD1602_D5_GPIO_Port, LCD1602_D5_Pin},
  {LCD1602_D6_GPIO_Port, LCD1602_D6_Pin},
  {LCD1602_D7_GPIO_Port, LCD1602_D7_Pin}
};

inline static void RS_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(LCD1602_RS_GPIO_Port, LCD1602_RS_Pin, pinState);
}

inline static void RW_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(LCD1602_RW_GPIO_Port, LCD1602_RW_Pin, pinState);
}

inline static void EN_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(LCD1602_E_GPIO_Port, LCD1602_E_Pin, pinState);
}

static void writeData(uint8_t data)
{
  // 读忙标志
  RS_Set(PIN_HIGH);
  RW_Set(PIN_LOW);
  Delay_Us(1);
  for(uint8_t i = 0; i < 8; i++){
    data >>= i;
    HAL_GPIO_WritePin(dataBit[i].port, dataBit[i].pin, (GPIO_PinState)(data & 0x01));
  }
  Delay_Us(1);
  EN_Set(PIN_HIGH);
  Delay_Us(1);
  EN_Set(PIN_LOW);
}

static void writeCmd(uint8_t cmd)
{
  // 读忙标志
  RS_Set(PIN_LOW);
  RW_Set(PIN_LOW);
  Delay_Us(1);
  for(uint8_t i = 0; i < 8; i++){
    cmd >>= i;
    HAL_GPIO_WritePin(dataBit[i].port, dataBit[i].pin, (GPIO_PinState)(cmd & 0x01));
  }
  Delay_Us(1);
  EN_Set(PIN_HIGH);
  Delay_Us(1);
  EN_Set(PIN_LOW);
}

static bool waitBusy(void)
{

}

void LCD1602_Init(void)
{
  writeCmd(LCD1602_FUNCTION_SET);
  writeCmd(LCD1602_FUNCTION_SET);
  writeCmd(LCD1602_DISPLAY_ON);
  writeCmd(LCD1602_CLEAR_DISPLAY);
  HAL_Delay(2);
  writeCmd(LCD1602_ENTRY_MODE_ADD);
  // writeCmd();
  // writeCmd();
}

void LCD1602_Test(void)
{
  writeData('A');
}