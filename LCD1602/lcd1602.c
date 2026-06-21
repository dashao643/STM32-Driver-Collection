#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "lcd1602.h"
#include "general.h"

#include <stdint.h>
#include <stdio.h>

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
  RS_Set(PIN_HIGH);
  RW_Set(PIN_LOW);
  for(uint8_t i = 0; i < 8; i++){
    HAL_GPIO_WritePin(dataBit[i].port, dataBit[i].pin, (GPIO_PinState)(data & 0x01));
    data >>= 1;
  }
  EN_Set(PIN_HIGH);
  Delay_Us(1);
  // HAL_Delay(1);
  EN_Set(PIN_LOW);
  Delay_Us(50);
  // HAL_Delay(1);
}

static void writeCmd(uint8_t cmd)
{
  RS_Set(PIN_LOW);
  RW_Set(PIN_LOW);
  for(uint8_t i = 0; i < 8; i++){
    HAL_GPIO_WritePin(dataBit[i].port, dataBit[i].pin, (GPIO_PinState)(cmd & 0x01));
    cmd >>= 1;
  }
  EN_Set(PIN_HIGH);
  Delay_Us(1);
  // HAL_Delay(1);
  EN_Set(PIN_LOW);
  Delay_Us(50);
  // HAL_Delay(1);
}

/*-----------------------------------------------------------------*/

void LCD1602_Init(void)
{
  writeCmd(LCD1602_FUNCTION_SET);     // 8位数据总线,2行显示,5*7点阵
  writeCmd(LCD1602_DISPLAY_ON);       // 显示开,光标不显示,不闪烁
  writeCmd(LCD1602_ENTRY_MODE_ADD);   // AC和光标自增,显示不移位

  LCD1602_Clear();
}

void LCD1602_Clear(void)
{
  writeCmd(LCD1602_CLEAR_DISPLAY);
  HAL_Delay(2);
}

void LCD1602_ShowAll(void)
{
  writeCmd(0x80);
  for(uint8_t i = 0; i < 16; i++){
    writeData(0xFF);
  }
  writeCmd(0xC0);
  for(uint8_t i = 0; i < 16; i++){
    writeData(0xFF);
  }
}
  
/**
 * @brief 指定某行的某列开始,显示字符串,行溢出丢弃
 * 
 * @param row 指定某行(1-2)
 * @param col 指定某列(1-16)
 * @param str 显示的字符串
 */
void LCD1602_ShowString(uint8_t row, uint8_t col, const char *str)
{
  if(row < 1 || row > 2) return;
  if(col < 1 || col > 16) return;

  uint8_t posCmd = 0x80; 
  if(row == 2) posCmd |= 0x40;

  writeCmd(posCmd + col - 1);

  uint8_t i = 0;
  while(str[i] != '\0') {
    if (col + i > 16) 
      return;
    writeData(str[i]);
    i++;
  }
}

/**
 * @brief 指定数字长度，以十进制数显示
 * 
 * @param row 指定某行(1-2)
 * @param col 指定某列(1-16)
 * @param num 32位有符号数
 * @param numLen 有符号数长度(所占位数,带负号)
 */
void LCD1602_ShowDecNum(uint8_t row, uint8_t col, int32_t num, uint8_t numLen)
{
  if(row < 1 || row > 2) return;
  if(col < 1 || col > 16) return;
  if (numLen == 0) return;  
  if (numLen > 11) numLen = 11;

  char buf[12] = {0};  // int32_t 最大值 + 负号 + \0

  snprintf(buf, sizeof(buf), "%0*d", numLen, (int)num);

  LCD1602_ShowString(row, col, buf);
}

/**
 * @brief 按字节,以十六进制数显示
 * 
 * @param row 指定某行(1-2)
 * @param col 指定某列(1-16)
 * @param data 原始字节数组
 * @param size 数组大小 1-5 Byte
 */
void LCD1602_ShowHexNum(uint8_t row, uint8_t col, const uint8_t *data, uint8_t size)
{
  if(row < 1 || row > 2) return;
  if(col < 1 || col > 16) return;
  if (size == 0) return;
  if (size > 5) size = 5;
  
  char buf[3] = {0};
  for(int i = 0; i < size; i++){
    snprintf(buf, sizeof(buf), "%0*X", 2, data[i]);
    LCD1602_ShowString(row, col + (i * 3), buf);
  }
}

/**
 * @brief 向DDRAM写入数据，配合move函数使用，最多一行写入40个字符
 * 
 * @param row row 指定某行(1-2)
 * @param str 显示的字符串
 */
void LCD1602_WriteDDRAM(uint8_t row, const char *str)
{
  if(row < 1 || row > 2) return;

  uint8_t posCmd = 0x80; 
  if(row == 2) posCmd |= 0x40;

  writeCmd(posCmd);

  uint8_t i = 0;
  while(str[i] != '\0') {
    if (i >= 40) 
      return;
    writeData(str[i]);
    i++;
  }
}

void LCD1602_MoveLeft(void)
{
  writeCmd(LCD1602_DISPLAY_SHIFT_L);
}

void LCD1602_MoveRight(void)
{
  writeCmd(LCD1602_DISPLAY_SHIFT_R);
}
