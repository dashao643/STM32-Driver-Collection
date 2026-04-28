#include "oled.h"
#include "oledfont.h"
#include "stm32f1xx_hal.h"
// #include "my_i2c.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "gpio.h"

/**
 * @brief 硬件I2C写命令
 * 
 * @param cmd 命令码
 */
static void OLED_WriteCmd(uint8_t cmd) 
{
#ifdef I2C_SOFTWARE
  I2C_Mem_Write(OLED_I2C_ADDR, OLED_CMD, 1, &cmd, 1);
#endif

#ifdef I2C_HARDWARE
  HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, OLED_CMD, 1, &cmd, 1, OLED_TIME_OUT); 
#endif
}

/**
 * @brief 硬件I2C写数据
 * 
 * @param data 字节数组
 * @param length 数据字节大小
 */
static void OLED_WriteData(uint8_t data[], uint16_t length) 
{ 
#ifdef I2C_SOFTWARE
  I2C_Mem_Write(OLED_I2C_ADDR, OLED_DATA, 1, data, length);
#endif

#ifdef I2C_HARDWARE
  HAL_I2C_Mem_Write(OLED_HANDLE, OLED_I2C_ADDR, OLED_DATA, 1, data, length, OLED_TIME_OUT); 
#endif
}

/**
 * @brief 初始化配置
 * 
 */
void OLED_Init(void) 
{
  uint8_t OLED_CmdInit[] = {
    0xAE, // 关闭显示
    0xD5, // 设置时钟分频因子,震荡频率
    0x80, // 分频因子=1,震荡频率=默认
    0xA8, // 设置多路复用率
    0x3F, // 1/64 Duty
    0xD3, // 设置显示偏移
    0x00, // 偏移0
    0x40, // 设置显示开始行
    0x8D, // 电荷泵设置
    0x14, // 开启电荷泵
    0x20, // 设置内存地址模式
    0x02, // 页面寻址模式
    0xA1, // 段重定义设置,SEG0->列0
    0xC8, // COM扫描方向,COM63->行0
    0xDA, // 设置COM硬件引脚配置
    0x12, // 
    0x81, // 对比度设置
    0x7F, // 对比度值
    0xD9, // 设置预充电周期
    0xF1, // 
    0xDB, // 设置VCOMH电压倍率
    0x40, // 
    0xA4, // 全局显示开启
    0xA6, // 正常显示
    0xAF, // 开启显示                    
  };
  HAL_Delay(100); // 上电延时

#ifdef I2C_SOFTWARE
  I2C_Mem_Write(OLED_I2C_ADDR, OLED_CMD, 1, 
                OLED_CmdInit, sizeof(OLED_CmdInit));
#endif

#ifdef I2C_HARDWARE
  HAL_I2C_Mem_Write(OLED_HANDLE, OLED_I2C_ADDR, OLED_CMD, 1, 
                    OLED_CmdInit, sizeof(OLED_CmdInit), OLED_TIME_OUT); 
#endif
  OLED_Clear();
}

void OLED_Clear(void) 
{
  uint8_t data[128] = {0};
  for (uint8_t i = 0; i < 8; i++) {
    OLED_WriteCmd(0xB0 + i);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteData(data, sizeof(data));
  }
}

void OLED_ShowALL(void) 
{
  uint8_t line[128] = {0};

  for (uint8_t i = 0; i < 8; i++) {
    OLED_WriteCmd(0xB0 + i);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    // memset按字节设置
    memset(line, 0xFF, sizeof(line));
    OLED_WriteData(line, sizeof(line));
  }
}

void OLED_SetReverse(void) 
{ 
  OLED_WriteCmd(0xA7); 
}

static void OLED_WriteCmdPos(uint8_t x, uint8_t y, uint8_t page_offs) 
{
  uint8_t page = (x - 1) * 2 + page_offs;
  uint8_t col = (y - 1) * 8;
  if (page > 7) 
    page = 7;
  if (col > 127) 
    col = 127;
  OLED_WriteCmd(0xB0 + page);         // 指定页地址
  OLED_WriteCmd(0x00 + (col & 0x0F)); // 列地址取低四位
  OLED_WriteCmd(0x10 + (col >> 4));   // 列地址取高四位
}

/**
 * @brief 显示字符
 * 
 * @param x 行号，1-4
 * @param y 列号，1-16
 * @param ch 字符
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch) 
{
  if (x == 0 || x > 4)
    return;
  if (y == 0 || y > 16)
    return;
  if ((ch < ASCII_OFFSET) || (ch > ASCII_LENGTH - 1 + ASCII_OFFSET))
    ch = ASCII_OFFSET; // 范围有误，显示为空字符

  uint8_t *aChar = (uint8_t *)OLED_ASCII_1608[ch - ASCII_OFFSET];

  OLED_WriteCmdPos(x, y, 0);
  OLED_WriteData(aChar, 8);

  OLED_WriteCmdPos(x, y, 1);
  OLED_WriteData(aChar + 8, 8);
}

// 显示字符串,创建字符串推荐不指定数组长度,或者手动加\0
void OLED_ShowString(uint8_t x, uint8_t y, char str[]) 
{
  if (str == NULL)
    return;
  if (x == 0 || x > 4)
    return;
  if (y == 0 || y > 16)
    return;

  uint8_t i = 0;
  while (str[i] != '\0') {
    if (y + i > 16)
      return;
    OLED_ShowChar(x, y + i, str[i]);
    i++;
  }
}

// 需要传入数据的位数，从低位开始显示，目的是更新显示区域 numLen: 1 - 11
void OLED_ShowDecNumber(uint8_t x, uint8_t y, int32_t number, uint8_t numLen) 
{
  if (x == 0 || x > 4)
    return;
  if (y == 0 || y > 16)
    return;
  if (numLen == 0 || numLen > 11)
    numLen = 11;

  char buf[12] = {0};  // int32_t 最大值 + 负号 + \0

  snprintf(buf, sizeof(buf), "%0*d", numLen, (int)number);

  OLED_ShowString(x, y, buf);
}

/**
 * @brief 按字节显示
 * 
 * @param x 行号，1-4
 * @param y 列号，1-16
 * @param data 原始字节数组
 * @param size 数组大小 1-5 Byte
 */
void OLED_ShowHexNumber(uint8_t x, uint8_t y, uint8_t data[], uint8_t size)
{
  if (x == 0 || x > 4)
    return;
  if (y == 0 || y > 16)
    return;

  char buf[3] = {0};
  for(int i = 0; i < size; i++){
    snprintf(buf, sizeof(buf), "%0*X", 2, data[i]);
    OLED_ShowString(x, y + (i * 3), buf);
  }
}
