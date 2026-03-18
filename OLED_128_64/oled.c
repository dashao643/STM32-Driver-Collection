#include "oled.h"
#include "oledfont.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "gpio.h"

#define OLED_I2C_ADDR 0x78
#define OLED_CMD      0x00 // 写命令
#define OLED_DATA     0x40 // 写数据

// 硬件I2C写命令/数据
static void OLED_WriteCmd(uint8_t cmd) { 
  HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, OLED_CMD, 1, &cmd, 1, 50); 
}

static void OLED_WriteData(uint8_t *data, uint8_t length) { 
  HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, OLED_DATA, 1, data, length, 50); 
}

void OLED_Init() {
  HAL_Delay(100); // 上电延时
  OLED_WriteCmd(0xAE); // 关闭显示
  OLED_WriteCmd(0xD5); // 设置时钟分频因子,震荡频率
  OLED_WriteCmd(0x80); // 分频因子=1,震荡频率=默认
  OLED_WriteCmd(0xA8); // 设置多路复用率
  OLED_WriteCmd(0x3F); // 1/64 Duty
  OLED_WriteCmd(0xD3); // 设置显示偏移
  OLED_WriteCmd(0x00); // 偏移0
  OLED_WriteCmd(0x40); // 设置显示开始行
  OLED_WriteCmd(0x8D); // 电荷泵设置
  OLED_WriteCmd(0x14); // 开启电荷泵
  OLED_WriteCmd(0x20); // 设置内存地址模式
  OLED_WriteCmd(0x02); // 页地址模式
  OLED_WriteCmd(0xA1); // 段重定义设置,SEG0->列0
  OLED_WriteCmd(0xC8); // COM扫描方向,COM63->行0
  OLED_WriteCmd(0xDA); // 设置COM硬件引脚配置
  OLED_WriteCmd(0x12); // 
  OLED_WriteCmd(0x81); // 对比度设置
  OLED_WriteCmd(0x7F); // 对比度值
  OLED_WriteCmd(0xD9); // 设置预充电周期
  OLED_WriteCmd(0xF1); // 
  OLED_WriteCmd(0xDB); // 设置VCOMH电压倍率
  OLED_WriteCmd(0x40); // 
  OLED_WriteCmd(0xA4); // 全局显示开启
  OLED_WriteCmd(0xA6); // 正常显示
  OLED_WriteCmd(0xAF); // 开启显示
  OLED_Clear();
}

void OLED_Clear() {
  uint8_t data[128] = {0};
  for (uint8_t i = 0; i < 8; i++) {
    OLED_WriteCmd(0xB0 + i);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteData(data, sizeof(data));
  }
}

void OLED_ShowALL() {
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

void OLED_SetReverse() { 
  OLED_WriteCmd(0xA7); 
}

static void OLED_WriteCmdPos(uint8_t x, uint8_t y, uint8_t page_offs) {
  uint8_t page = (x - 1) * 2 + page_offs;
  uint8_t col = (y - 1) * 8;
  if (page > 7) 
    page = 7;
  if (col > 127) 
    col = 127;
  OLED_WriteCmd(0xB0 + page);
  OLED_WriteCmd(0x00 + (col & 0x0F)); // 取低四位
  OLED_WriteCmd(0x10 + (col >> 4));   // 取高四位
}

void OLED_Show_Char(uint8_t x, uint8_t y, char ch) {
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

// 显示字符串,创建字符串推荐手动加\0,或者不要指定数组长度
void OLED_Show_String(uint8_t x, uint8_t y, uint8_t *str) {
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
    OLED_Show_Char(x, y + i, str[i]);
    i++;
  }
}
//此函数需要传入数据的位数，从低位开始显示，目的是更新显示区域 maxLen: 1 - 11
void OLED_Show_DecNumber(uint8_t x, uint8_t y, int32_t number, uint8_t maxLen) {
  if (x == 0 || x > 4)
    return;
  if (y == 0 || y > 16)
    return;
  if (maxLen == 0 || maxLen > 11)
    maxLen = 11;

  uint8_t buf[12] = {0};  // int32_t 最大值 + 负号 + \0

  snprintf(buf, sizeof(buf), "%0*d", maxLen, (int)number);

  OLED_Show_String(x, y, buf);
}
