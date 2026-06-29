#include "stm32f4xx_hal.h"
#include "st7735.h"
#include "st7735_def.h"
#include "st7735_font.h"
#include "spi.h"
#include "general.h"

#include <stdint.h>
#include <string.h>

/*
The MSB is transmitted first
*/

// PIN_HIGH / PIN_LOW
// 低有效
inline static void RES_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, pinState);
}

// 低有效(外部非静态函数调用)
inline static void CS_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, pinState);
}

// low: cmd high: data (或命令参数)
inline static void DC_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, pinState);
}

static void writeData(uint8_t data)
{
  DC_Set(PIN_HIGH);
  HAL_SPI_Transmit(ST7735_HANDLE, &data, 1, ST7735_TX_TIMEOUT_MS);
}

static void writeData16Bit(uint16_t data)
{
  uint8_t pData[2] = {data >> 8, data};

  DC_Set(PIN_HIGH);
  HAL_SPI_Transmit(ST7735_HANDLE, pData, 2, ST7735_TX_TIMEOUT_MS);
}

static void writeCmd(uint8_t cmd)
{
  DC_Set(PIN_LOW);
  HAL_SPI_Transmit(ST7735_HANDLE, &cmd, 1, ST7735_TX_TIMEOUT_MS);
}

static void setStartPoint(uint8_t row, uint8_t col)
{
  if(row > 159) row = 159;
  if(col > 127) col = 127;

	writeCmd(0x2A);
	writeData(0x00);
	writeData(col);
	writeData(0x00);
	writeData(0x7F);

	writeCmd(0x2B);
	writeData(0x00);
	writeData(row);
	writeData(0x00);
	writeData(0x9F);
}

void ST7735_Init(void)
{
/********************* 引脚初始状态 *********************/
  RES_Set(PIN_HIGH);
  CS_Set(PIN_HIGH);
  DC_Set(PIN_LOW);

/********************* 上电延时 *********************/
  HAL_Delay(50);

/********************* 开始通信 *********************/
  CS_Set(PIN_LOW);

/********************* RES复位 *********************/
	RES_Set(PIN_LOW);
  Delay_Us(10);
	RES_Set(PIN_HIGH);
	HAL_Delay(5);

/********************* 初始化指令 *********************/
	writeCmd(0x11);   // sleep out 退出睡眠模式 
	HAL_Delay(120);

  // writeCmd(0x01);   // 软件复位 + 进入睡眠?
  // HAL_Delay(120);
  // writeCmd(0x10);   // 进入睡眠模式
  // writeCmd(0x13);   // Normal Display Mode On
  // writeCmd(0x20);   // Display Inversion Off
  // writeCmd(0x21);   // Display Inversion On
  writeCmd(0x29);   // Display On

  writeCmd(0x2A);   // 设置列地址窗口
	writeData(0x00);
	writeData(0x00);
	writeData(0x00);
	writeData(0x7F);

	writeCmd(0x2B);   // 设置行地址窗口
	writeData(0x00);
	writeData(0x00);
	writeData(0x00);
	writeData(0x9F);

  // writeCmd(0x2C);    // Memory Write
  // writeData(0x33):  // Scroll Area Set 
	writeCmd(0x36);    // MX, MY, RGB mode 
	writeData(0x00); 

  // writeCmd(0x37);     // Vertical Scroll Start Address of RAM
  writeCmd(0x3A);  // Interface Pixel Format 
  writeData(0x05);  // 16-bit/pixel 

  writeCmd(0xB1); //  Panel Function Command List 
	writeData(0x01); 
	writeData(0x2C); 
	writeData(0x2D); 

	writeCmd(0xB4); //Column inversion 
	writeData(0x07); 

	writeCmd(0xC0); 
	writeData(0xA2); 
	writeData(0x02); 
	writeData(0x84); 

	writeCmd(0xC1); 
	writeData(0xC5); 

	writeCmd(0xC2); 
	writeData(0x0A); 
	writeData(0x00); 

	writeCmd(0xC5); //VCOM 
	writeData(0x0E); 

  CS_Set(PIN_HIGH);

/********************* 清屏指令 *********************/
  ST7735_Clear();
}

// 40960 Byte -> 41KB
void ST7735_Clear(void)
{
  CS_Set(PIN_LOW);
  setStartPoint(0, 0);
  writeCmd(0x2C);
  DC_Set(PIN_HIGH);

  uint8_t rowBuf[ST7735_COL * 2] = {0};
  memset(rowBuf, ST7735_WHITE, sizeof(rowBuf));
  
  for(uint8_t row = 0; row < ST7735_ROW; row++){
    HAL_SPI_Transmit(ST7735_HANDLE, rowBuf, sizeof(rowBuf), ST7735_TX_TIMEOUT_MS);
  }
  CS_Set(PIN_HIGH);
}

void ST7735_Test(void)
{
  CS_Set(PIN_LOW);

  setStartPoint(0, 0);
  writeCmd(0x2C);
  
  for(uint16_t i = 0; i < 1000; i++){
    writeData16Bit(0xFFE0);
  }

  CS_Set(PIN_HIGH);
}
