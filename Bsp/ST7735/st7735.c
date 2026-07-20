#include "stm32f4xx_hal.h"
#include "st7735.h"
#include "st7735_def.h"
#include "st7735_font.h"
#include "spi.h"
#include "general.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/*
128 * 160
RGB: 5 6 5 = 16
默认为黑屏
正常初始化后屏幕显示为黑屏且透光
*/

inline static void RES_Set(GPIO_PinState pinState);
inline static void CS_Set(GPIO_PinState pinState);
inline static void DC_Set(GPIO_PinState pinState);
static void writeData(uint8_t data);
static void writeCmd(uint8_t cmd);
static void setStartPixel(uint8_t rowPixel, uint8_t colPixel);
static void setPixelWindow(uint8_t rowBegin, uint8_t colBegin, uint8_t rowEnd, uint8_t colEnd);

// 低有效
inline static void RES_Set(GPIO_PinState pinState)
{
 	HAL_GPIO_WritePin(ST7735_RST_GPIO_Port, ST7735_RST_Pin, pinState);
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

static void writeCmd(uint8_t cmd)
{
	DC_Set(PIN_LOW);
	HAL_SPI_Transmit(ST7735_HANDLE, &cmd, 1, ST7735_TX_TIMEOUT_MS);
}

// row: 0 - 159, col: 0 - 127
static void setStartPixel(uint8_t rowPixel, uint8_t colPixel)
{
	if(rowPixel >= ST7735_ROW_PIXEL) return;
	if(colPixel >= ST7735_COL_PIXEL) return;

	writeCmd(ST7735_COLUMN_ADDRESS_SET);
	writeData(0x00);
	writeData(colPixel);
	writeData(0x00);
	writeData(ST7735_COL_PIXEL - 1);

	writeCmd(ST7735_ROW_ADDRESS_SET);
	writeData(0x00);
	writeData(rowPixel);
	writeData(0x00);
	writeData(ST7735_ROW_PIXEL - 1);
}

static void setPixelWindow(uint8_t rowBegin, uint8_t colBegin, uint8_t rowEnd, uint8_t colEnd)
{
	if(rowBegin >= ST7735_ROW_PIXEL) return;
	if(colBegin >= ST7735_COL_PIXEL) return;
	if(rowEnd >= ST7735_ROW_PIXEL) return;
	if(colEnd >= ST7735_COL_PIXEL) return;

	writeCmd(ST7735_COLUMN_ADDRESS_SET);
	writeData(0x00);
	writeData(colBegin);
	writeData(0x00);
	writeData(colEnd);

	writeCmd(ST7735_ROW_ADDRESS_SET);
	writeData(0x00);
	writeData(rowBegin);
	writeData(0x00);
	writeData(rowEnd);
}

/*-----------------------------------------------------------------*/

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
	// RES_Set(PIN_LOW);
	// Delay_Us(10);
	// RES_Set(PIN_HIGH);
	// HAL_Delay(5);

/********************* 初始化指令 *********************/
	writeCmd(ST7735_SLEEP_OUT);
	HAL_Delay(120);

 	writeCmd(ST7735_NORMAL_DISPLAY_MODE_ON);

 	writeCmd(ST7735_DISPLAY_INVERSION_OFF);

 	writeCmd(ST7735_DISPLAY_ON);

 	writeCmd(ST7735_COLUMN_ADDRESS_SET);
	writeData(0x00);  // begin
	writeData(0x00);
	writeData(0x00);  // end
	writeData(0x7F);

	writeCmd(ST7735_ROW_ADDRESS_SET);
	writeData(0x00);  // begin
	writeData(0x00);
	writeData(0x00);
	writeData(0x9F);  // end

	writeCmd(ST7735_MEMORY_DATA_ACCESS_CONTROL);
	writeData(0xC0);  // MY MX MV ML RGB MH

	writeCmd(ST7735_INTERFACE_PIXEL_FORMAT);
	writeData(0x05);  // 16-bit/pixel 

	writeCmd(ST7735_FRAME_RATE_CONTROL);
	writeData(0x01); 
	writeData(0x2C); 
	writeData(0x2D); 

	writeCmd(ST7735_DISPLAY_INVERSION_CONTROL);
	writeData(0x07); // Dot Inversion

	writeCmd(ST7735_POWER_CONTROL_1); 
	writeData(0xA2); 
	writeData(0x02); 
	writeData(0x84); 

	writeCmd(ST7735_POWER_CONTROL_2); 
	writeData(0xC5); 

	writeCmd(ST7735_POWER_CONTROL_3); 
	writeData(0x0A); 
	writeData(0x00); 

	writeCmd(ST7735_VCOM_CONTROL_1);
	writeData(0x0E); 

	writeCmd(ST7735_GAMMA_POS_POLARITY);
	writeData(0x0f); 
	writeData(0x1a); 
	writeData(0x0f); 
	writeData(0x18); 
	writeData(0x2f); 
	writeData(0x28); 
	writeData(0x20); 
	writeData(0x22); 
	writeData(0x1f); 
	writeData(0x1b); 
	writeData(0x23); 
	writeData(0x37); 
	writeData(0x00); 	
	writeData(0x07); 
	writeData(0x02); 
	writeData(0x10); 

	writeCmd(ST7735_GAMMA_NEG_POLARITY);
	writeData(0x0f); 
	writeData(0x1b); 
	writeData(0x0f); 
	writeData(0x17); 
	writeData(0x33); 
	writeData(0x2c); 
	writeData(0x29); 
	writeData(0x2e); 
	writeData(0x30); 
	writeData(0x30); 
	writeData(0x39); 
	writeData(0x3f); 
	writeData(0x00); 
	writeData(0x07); 
	writeData(0x03); 
	writeData(0x10);  

	CS_Set(PIN_HIGH);

/********************* 清屏指令 *********************/
	ST7735_Clear();
}

// 40960 Byte -> 41KB
void ST7735_Clear(void)
{
	CS_Set(PIN_LOW);
	setStartPixel(0, 0);
	writeCmd(0x2C);
	DC_Set(PIN_HIGH);

	// 清屏显示为黑色
	uint8_t rowBuf[ST7735_COL_PIXEL * 2] = {0};
	// memset(rowBuf, 0xFF, sizeof(rowBuf));
	
	for(uint8_t row = 0; row < ST7735_ROW_PIXEL; row++){
		HAL_SPI_Transmit(ST7735_HANDLE, rowBuf, sizeof(rowBuf), ST7735_TX_TIMEOUT_MS);
	}
	CS_Set(PIN_HIGH);
}

/**
 * @brief 指定行列显示字符
 * 
 * @param row row 指定在 1 - 10 行显示
 * @param col col 指定在 1 - 16 列显示
 * @param ch str 字符串数组
 * @param color 16位RGB(5,6,5)
 */
void ST7735_ShowChar(uint8_t row, uint8_t col, char ch, uint16_t color)
{
	if(row == 0 || row > ST7735_ROW_CNT) return;
	if(col == 0 || col > ST7735_COL_CNT) return;

	CS_Set(PIN_LOW);

	uint8_t rowBegin = (row - 1) * CHARACTER_HEIGHT;
	uint8_t colBegin = (col - 1) * CHARACTER_WIDTH;
	
	uint8_t *chArr = (uint8_t *)ST7735_ASCII_1608[ch - ASCII_OFFSET];

	// 256 字节单字符缓冲,初始化为为黑色
	static uint8_t rowBuf[CHARACTER_WIDTH * CHARACTER_HEIGHT * 2];
	memset(rowBuf, 0x00, sizeof(rowBuf));

	// 设置单个字符显示窗口大小
	setPixelWindow(rowBegin, colBegin, rowBegin + CHARACTER_HEIGHT - 1, colBegin + CHARACTER_WIDTH - 1);

	// 根据字符数组填充缓冲
	for (uint8_t byteIdx = 0; byteIdx < 16; byteIdx++) {
		uint8_t chByte = chArr[byteIdx];

		for(uint8_t bitIdx = 0; bitIdx < 8; bitIdx++){
		uint16_t pixelIdx = byteIdx * 8 + bitIdx;
		if(chByte & 0x80) {
			rowBuf[pixelIdx * 2] = color >> 8;
			rowBuf[pixelIdx * 2 + 1] = color;
		}
			chByte <<= 1;
		}
	}

	writeCmd(ST7735_MEMORY_WRITE);
	DC_Set(PIN_HIGH);
	HAL_SPI_Transmit(ST7735_HANDLE, rowBuf, sizeof(rowBuf), ST7735_TX_TIMEOUT_MS);

	CS_Set(PIN_HIGH);
}

/**
 * @brief 指定行列显示字符串(Z型显示,溢出屏幕丢弃)
 * 
 * @param row row 指定在 1 - 10 行显示
 * @param col col 指定在 1 - 16 列显示
 * @param str str 字符串数组
 * @param color 16位RGB(5,6,5)
 */
void ST7735_ShowString(uint8_t row, uint8_t col, const char *str, uint16_t color)
{
	if(row == 0 || row > ST7735_ROW_CNT) return;
	if(col == 0 || col > ST7735_COL_CNT) return;

	uint16_t length = strlen(str);

	for(uint8_t i = 0; i < length; i++){
		ST7735_ShowChar(row, col++, str[i], color);
		if(col > ST7735_COL_CNT) {
			col = 1;
			row++;
		}
	}
}

void ST7735_ShowDecNumber(uint8_t row, uint8_t col, int32_t number, uint8_t numLen) 
{
	if(row == 0 || row > ST7735_ROW_CNT) return;
	if(col == 0 || col > ST7735_COL_CNT) return;
	if (numLen == 0) return;  
	if (numLen > 11) numLen = 11;
		
	char buf[12] = {0};  // int32_t 最大值 + 负号 + \0

	snprintf(buf, sizeof(buf), "%0*d", numLen, (int)number);

	ST7735_ShowString(row, col, buf, ST7735_WHITE);
}

// 传输时长: 9ms一张, 支持 128_X_160 和 128_X_128
// 图片格式: MSB First
void ST7735_ShowImage(const uint8_t *image, uint16_t imageWindow)
{
	uint16_t arrSize = imageWindow;

  CS_Set(PIN_LOW);

	if(imageWindow == ST7735_IMAGE_128_X_160) {
		// arrSize = 40960; // (160 * 128 * 2)
		setStartPixel(0,0);
	}
	else if(imageWindow == ST7735_IMAGE_128_X_128) {
		// arrSize = 32768;	// (128 * 128 * 2)
		// 正方形图像,正中间显示
		setPixelWindow(16, 0, 143, 127);
	}
	writeCmd(ST7735_MEMORY_WRITE);
	DC_Set(PIN_HIGH);
	HAL_SPI_Transmit(ST7735_HANDLE, image, arrSize, ST7735_TX_TIMEOUT_MS);

  CS_Set(PIN_HIGH);
}

/**
 * @brief 流式传输显示图片
 * 
 * @param image 图片数值
 * @param curTimes 当前的次数(1-160)
 * @param sumTimes 总次数(1-160)
 *	@note 最多分为160次显示,一次最少显示一行
 * @param imageWindow 图片分辨率窗口
 */

void ST7735_ImageStream(const uint8_t *image, uint8_t curTimes, uint8_t sumTimes, uint16_t imageWindow)
{
	if(curTimes == 0 || curTimes > ST7735_ROW_PIXEL) return;
	if(sumTimes == 0 || sumTimes > ST7735_ROW_PIXEL) return;
	if(curTimes > sumTimes) return;

	// uint8_t rowBegin = 0;

	// uint16_t arrSize = imageWindow / sumTimes;

  // CS_Set(PIN_LOW);

	// if(imageWindow == ST7735_IMAGE_160_X_128) {
	// 	// arrSize = 40960; // (160 * 128 * 2)
	// 	setStartPixel(0,0);
	// }
	// else if(imageWindow == ST7735_IMAGE_128_X_128) {
	// 	// arrSize = 32768;	// (128 * 128 * 2)
	// 	// 正方形图像,正中间显示
	// 	setPixelWindow(16, 0, 143, 127);
	// }
	// else if(imageWindow == ST7735_IMAGE_128_X_160) {
	// 	// arrSize = 40960;
	// 	// 修改显示方向
	// }
	// writeCmd(ST7735_MEMORY_WRITE);
	// DC_Set(PIN_HIGH);
	// HAL_SPI_Transmit(ST7735_HANDLE, image, arrSize, ST7735_TX_TIMEOUT_MS);

  // CS_Set(PIN_HIGH);
}

// 固定一次传输W25Q64的一个扇区数据(4096B)
void ST7735_ImageStreamFix(const uint8_t *image, uint8_t curTimes, uint16_t imageWindow)
{
	uint8_t sumTimes = 0;
	uint8_t beginRow = 0;

	if(imageWindow == ST7735_IMAGE_128_X_160) {
		sumTimes = 10;
		beginRow = 0 + (curTimes - 1) * 16;
	}
	else if (imageWindow == ST7735_IMAGE_128_X_128) {
		sumTimes = 8;
		beginRow = 16 + (curTimes - 1) * 16;
	}

	if(curTimes == 0 || curTimes > sumTimes) return;

  CS_Set(PIN_LOW);

	setStartPixel(beginRow, 0);

	writeCmd(ST7735_MEMORY_WRITE);
	DC_Set(PIN_HIGH);
	HAL_SPI_Transmit(ST7735_HANDLE, image, 4096, ST7735_TX_TIMEOUT_MS);

  CS_Set(PIN_HIGH);
}

void ST7735_Scroll(void)
{
	// writeCmd(ST7735_SCROLL_AREA_SET);
	// writeData(0x00);
	// writeData(0x02);
	// writeData(0x00);
	// writeData(0xA0);
	// writeData(0x00);
	// writeData(0x00);

	// writeCmd(ST7735_VERTICAL_SCROLL_START);
	// writeData(0x00);
	// writeData(0x50);
	// writeCmd(ST7735_DISPLAY_INVERSION_CONTROL);
}