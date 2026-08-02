#include "stm32f4xx_hal.h"
#include "oled.h"
#include "oled_font.h"

#if defined I2C_SOFTWARE
#include "my_i2c.h"
#elif defined I2C_HARDWARE
#include "i2c.h"
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// static void OLED_WriteCmd(uint8_t *cmd, uint8_t size);
static void OLED_WriteCmd(uint8_t cmd);
static void OLED_WriteData(const uint8_t data[], uint16_t length);
static void OLED_WriteCmdPos(uint8_t row, uint8_t col, uint8_t pageOffs);

static void OLED_WriteCmd(uint8_t cmd) 
{
#ifdef I2C_SOFTWARE
  I2C_Mem_Write(OLED_I2C_SLAVE_ADDR, OLED_I2C_CMD, 1, &cmd, 1);
#endif

#ifdef I2C_HARDWARE
  HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_SLAVE_ADDR, OLED_I2C_CMD, 
                    1, &cmd, 1, OLED_TIME_OUT); 
#endif
}

/**
 * @brief 先发送写数据指令，再发送具体命令
 * 
 * @param data 字节数组
 * @param length 数据字节大小
 */
static void OLED_WriteData(const uint8_t data[], uint16_t length) 
{ 
#ifdef I2C_SOFTWARE
  I2C_Mem_Write(OLED_I2C_SLAVE_ADDR, OLED_I2C_DATA, 1, data, length);
#endif

#ifdef I2C_HARDWARE
  HAL_I2C_Mem_Write(OLED_HANDLE, OLED_I2C_SLAVE_ADDR, OLED_I2C_DATA, 
                    1, (uint8_t *)data, length, OLED_TIME_OUT); 
#endif
}

static void OLED_WriteCmdPos(uint8_t row, uint8_t col, uint8_t pageOffs) 
{
  uint8_t page = (row - 1) * 2 + pageOffs;
  uint8_t colIdx = (col - 1) * 8;

  if (page > 7)  page = 7;
  if (colIdx > 127)  colIdx = 127;

  OLED_WriteCmd(0xB0 + page);         // 指定页地址
  OLED_WriteCmd(0x00 + (colIdx & 0x0F)); // 列地址取低四位
  OLED_WriteCmd(0x10 + (colIdx >> 4));   // 列地址取高四位
}

/*-----------------------------------------------------------------*/

void OLED_Init(void) 
{
  HAL_Delay(100);

  OLED_WriteCmd(0xAE);    // 关闭显示
  OLED_WriteCmd(0xD5);    // 设置时钟分频因子,震荡频率
  OLED_WriteCmd(0x80);    // 分频因子=1,震荡频率=默认
  OLED_WriteCmd(0xA8);    // 设置多路复用率
  OLED_WriteCmd(0x3F);    // 1/64 Duty
  OLED_WriteCmd(0xD3);    // 设置显示偏移
  OLED_WriteCmd(0x00);    // 偏移0
  OLED_WriteCmd(0x40);    // 设置显示开始行
  OLED_WriteCmd(0x8D);    // 电荷泵设置
  OLED_WriteCmd(0x14);    // 开启电荷泵
  OLED_WriteCmd(0x20);    // 设置内存地址模式
  OLED_WriteCmd(0x02);    // 页面寻址模式
  OLED_WriteCmd(0xA1);    // 段重定义设置,SEG0->列0
  OLED_WriteCmd(0xC8);    // COM扫描方向,COM63->行0
  OLED_WriteCmd(0xDA);    // 设置COM硬件引脚配置
  OLED_WriteCmd(0x12);    // 
  OLED_WriteCmd(0x81);    // 对比度设置
  OLED_WriteCmd(0x7F);    // 对比度值
  OLED_WriteCmd(0xD9);    // 设置预充电周期
  OLED_WriteCmd(0xF1);    // 
  OLED_WriteCmd(0xDB);    // 设置VCOMH电压倍率
  OLED_WriteCmd(0x40);    // 
  OLED_WriteCmd(0xA4);    // 全局显示开启
  OLED_WriteCmd(0xA6);    // 正常显示
  OLED_WriteCmd(0xAF);    // 开启显示          

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

/**
 * @brief 显示字符
 * 
 * @param row 行号，1-4
 * @param col 列号，1-16
 * @param ch 字符
 */
void OLED_ShowChar(uint8_t row, uint8_t col, char ch) 
{
  if (row == 0 || row > 4) return;
  if (col == 0 || col > 16) return;

  uint8_t *chIdx;

  // 范围有误，显示为 ◼ 
  if ((ch < ASCII_OFFSET) || (ch > ASCII_LENGTH - 1 + ASCII_OFFSET))
    chIdx = (uint8_t *)OLED_ERROR_ASCII;    
  else
    chIdx = (uint8_t *)OLED_ASCII_08x16[ch - ASCII_OFFSET];

  OLED_WriteCmdPos(row, col, 0);
  OLED_WriteData(chIdx, 8);

  OLED_WriteCmdPos(row, col, 1);
  OLED_WriteData(chIdx + 8, 8);
}

// 显示字符串,创建字符串推荐不指定数组长度,或者手动加\0
void OLED_ShowString(uint8_t row, uint8_t col, const char str[]) 
{
  if (str == NULL) return;
  if (row == 0 || row > 4) return;
  if (col == 0 || col > 16) return;

  uint8_t i = 0;

  while (str[i] != '\0') {
    if (col + i > 16) return;

    OLED_ShowChar(row, col + i, str[i]);
    i++;
  }
}

void OLED_ShowFonts(uint8_t row, uint8_t col, const char font[])
{
  if (font == NULL) return;
  if (row == 0 || row > 4) return;
  if (col == 0 || col > 16) return;

  while(*font != '\0') {
    if(col > 16) return;

    // 先看第 bit7 是否为 0, 若为 0 则以 ascii 字符显示
    if(!(*font & 0x80)) {
      OLED_ShowChar(row, col, *font);
      col++;
      font++;
      continue;
    }

    uint8_t cellCnt = 0;

#if defined OLED_FONT_UTF8
    // 统计 utf8 中文所占字节数
    for(uint8_t i = 4; i < 8; i++) {
      if ((font[0] >> i) & 0x01) {
        cellCnt = 8 - i;
        break;
      }
    }
#elif defined OLED_FONT_GB2312
    // GB2312 中文字节数固定为2
    cellCnt = 2;
#endif

    bool find = false;
    for(uint8_t i = 0; i < CHINESE_FONT_COUNT; i++) {
      // 此文字匹配数组, 显示
      if(memcmp(font, OLED_CHINESE_16x16[i].index, cellCnt) == 0) {
        OLED_WriteCmdPos(row, col, 0);
        OLED_WriteData(OLED_CHINESE_16x16[i].cell, 16);

        OLED_WriteCmdPos(row, col, 1);
        OLED_WriteData(OLED_CHINESE_16x16[i].cell + 16, 16);

        find = true;
        break;
      }
    }
    // 没找到文字, 显示为 ■
    if(!find) {
      OLED_WriteCmdPos(row, col, 0);
      OLED_WriteData(OLED_ERROR_FONT, 16);

      OLED_WriteCmdPos(row, col, 1);
      OLED_WriteData(OLED_ERROR_FONT + 16, 16);
    }
    // font 指针偏移 cellCnt 字节
    font += cellCnt;
    // 移动列指针
    col += 2;
  }
}

// 需要传入数据的位数，从低位开始显示，目的是更新显示区域 numLen: 1 - 11
void OLED_ShowDecNumber(uint8_t row, uint8_t col, int32_t number, uint8_t numLen) 
{
  if (row == 0 || row > 4) return;
  if (col == 0 || col > 16) return;
  if (numLen == 0) return;  
  if (numLen > 11) numLen = 11;
    
  char buf[12] = {0};  // int32_t 最大值 + 负号 + \0

  snprintf(buf, sizeof(buf), "%0*d", numLen, (int)number);

  OLED_ShowString(row, col, buf);
}

/**
 * @brief 按字节显示
 * 
 * @param row 行号，1-4
 * @param col 列号，1-16
 * @param data 原始字节数组
 * @param size 数组大小 1-5 Byte
 */
void OLED_ShowHexNumber(uint8_t row, uint8_t col, const uint8_t data[], uint8_t size)
{
  if (row == 0 || row > 4) return;
  if (col == 0 || col > 16) return;
  if (size == 0) return;
  if (size > 5) size = 5;

  char buf[3] = {0};
  for(int i = 0; i < size; i++){
    snprintf(buf, sizeof(buf), "%02X", data[i]);
    OLED_ShowString(row, col + (i * 3), buf);
  }
}

// 一次性发送,连续写入一行(会清空一行中原有数据,适合按行显示的长数据)(经过测试, 耗时时间相同)
void OLED_StrWriteLine(uint8_t row, const char str[])
{
  uint8_t topRow[128] = {0};
  uint8_t bottomRow[128] = {0};

  for(uint8_t col = 0; col < 16; col++){
    char ch = str[col];
    if(ch == 0) 
      break;

    uint8_t index = col * 8;
    // 放上半字符
    for(uint8_t j = 0; j < 8; j++){
      topRow[index + j] = OLED_ASCII_08x16[ch - ASCII_OFFSET][j];
    }
    // 放下半字符
    for(uint8_t j = 0; j < 8; j++){
      bottomRow[index + j] = OLED_ASCII_08x16[ch - ASCII_OFFSET][j + 8];
    }
  }

  OLED_WriteCmdPos(row, 0, 0);
  OLED_WriteData(topRow, 128);
  OLED_WriteCmdPos(row, 0, 1);
  OLED_WriteData(bottomRow, 128);
}

void OLED_ShowImage(const uint8_t *image)
{
  for (uint8_t row = 0; row < 8; row++) {
    OLED_WriteCmd(0xB0 + row);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);

    OLED_WriteData(image + row * 128, 128);
  }
}