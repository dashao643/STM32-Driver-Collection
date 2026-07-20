#include "stm32f4xx_hal.h"
#include "ov7670.h"
#include "ov7670_def.h"
#include "sccb.h"
#include "dcmi.h"
#include "tim.h"
#include "general.h"

#include <stdint.h>
#include <stdio.h>

/*
640X480

图像分辨率类型:
CIF: 352 * 288
QVGA(Quarter-VGA): 320 * 240
VGA: 640 * 480
QCIF: 176 * 144

VSYNC: VSYNC一个上跳沿加一个下跳沿表示一个开始/结束信号, 有效电平为高电平
HREF: 默认使用HREF, 不使用HSYNC, 有效电平为低电平
PCLK: PCLK上升沿读取 D0~D7, PCLK上升沿 D0~D7 电平跳变
*/

static uint32_t frame[OV7670_FRAME_WORD_BUFF] = {0};

static inline HAL_StatusTypeDef regWrite(uint8_t regAddr, uint8_t data);
static inline HAL_StatusTypeDef regRead(uint8_t regAddr, uint8_t* data);
static void configWindow(uint16_t startx, uint16_t starty, uint16_t width, uint16_t height) UNUSED_FUNC;

static inline HAL_StatusTypeDef regWrite(uint8_t regAddr, uint8_t data)
{
  return SCCB_Mem_Write(OV7670_SLAVE_ADDR, regAddr, data);
}

static inline HAL_StatusTypeDef regRead(uint8_t regAddr, uint8_t* data)
{
  return SCCB_Mem_Read(OV7670_SLAVE_ADDR, regAddr, data);
}

static void configWindow(uint16_t startx, uint16_t starty, uint16_t width, uint16_t height)
{
	uint16_t endx = startx + width * 2;
	uint16_t endy = starty + height * 2;

	uint8_t tempReg1, tempReg2;

	regRead(OV7670_REG_VREF, &tempReg1);
	tempReg1 &= 0xF0;   // 保留高4位
	regRead(OV7670_REG_HREF, &tempReg2);
	tempReg2 &= 0xC0;   // 保留高2位

	// Horizontal
	uint8_t temp = tempReg2 | ((endx & 0x07) << 3) | (startx & 0x07);
	regWrite(0x32, temp);
	temp = (startx >> 3) & 0x7F;
	regWrite(0x17, temp);
	temp = (endx >> 3) & 0x7F;
	regWrite(0x18, temp);

	// Vertical
	temp = tempReg1 | ((endy & 0x03) << 2) | (starty & 0x03);
	regWrite(0x03, temp);
	temp = starty >> 2;
	regWrite(0x19, temp);
	temp = endy >> 2;
	regWrite(0x1A, temp);
}

/*-----------------------------------------------------------------*/

void OV7670_Init(void)
{
  /********************* 硬件复位 *********************/
  HAL_GPIO_WritePin(OV7670_RES_GPIO_Port, OV7670_RES_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(OV7670_RES_GPIO_Port, OV7670_RES_Pin, GPIO_PIN_SET);

  /********************* 提供 XCLK 时钟信号, 等待稳定 *********************/
  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
  HAL_Delay(200);

  /********************* SCCB 通信校验 *********************/
  uint8_t pid = 0, ver = 0, com5 = 0;
  uint8_t state1 = regRead(OV7670_REG_PID, &pid);
  uint8_t state2 = regRead(OV7670_REG_VER, &ver);
  printf("state1=%d, state2=%d\n", state1, state2);
  printf("OV7670 PID=0x%02X VER=0x%02X\n", pid, ver);

  if(pid != OV7670_PID_VAL || ver != OV7670_VER_VAL) {
    printf("ov7670 read fail\n");
    return;
  }
  regWrite(OV7670_REG_COM5, 0x55);
  regRead(OV7670_REG_COM5, &com5);
  if(com5 != 0x55) {
    printf("ov7670 write fail\n");
    return;
  }

  /********************* 寄存器初始化 *********************/
  // 软件复位
  regWrite(OV7670_REG_COM7, 0x80);
  HAL_Delay(50);

  // 颜色通道增益
  regWrite(OV7670_REG_BLUE, 0x80);
  regWrite(OV7670_REG_RED, 0x80);

  // 窗口设置
  // regWrite(OV7670_REG_VREF, 0x0A);
  // regWrite(OV7670_REG_HSTART, 0x16);
  // regWrite(OV7670_REG_HSTOP, 0x04);
  // regWrite(OV7670_REG_HREF, 0x80);
  // regWrite(OV7670_REG_VSTRT, 0x03);
  // regWrite(OV7670_REG_VSTOP, 0x7B);

  regWrite(OV7670_REG_COM2, 0x03);
  regWrite(OV7670_REG_COM3, 0x04);
  regWrite(OV7670_REG_COM4, 0x00);
  regWrite(OV7670_REG_COM6, 0x61);

  regWrite(OV7670_REG_AECH, 0x00);
  regWrite(OV7670_REG_CLKRC, 0xFF);

  // QVGA + RGB 输出
  regWrite(OV7670_REG_COM7, 0x14);
  regWrite(OV7670_REG_COM8, 0xFF);
  regWrite(OV7670_REG_COM9, 0x4E);

  // 时钟极性默认设置
  regWrite(OV7670_REG_COM10, 0x00);

  // 水平, 竖直翻转
  regWrite(OV7670_REG_MVFP, 0x37);

  regWrite(OV7670_REG_AEW, 0x75);
  regWrite(OV7670_REG_AEB, 0x63);
  regWrite(OV7670_REG_VPT, 0xA5);
  
  regWrite(OV7670_REG_CHLF, 0x0B);
  regWrite(OV7670_REG_ARBLM, 0x11);
  
  regWrite(OV7670_REG_ADC, 0x3F);
  regWrite(OV7670_REG_ACOM, 0x01);
  regWrite(OV7670_REG_OFON, 0x00);

  regWrite(OV7670_REG_TSLB, 0x04);
  regWrite(OV7670_REG_COM11, 0x02);

	regWrite(OV7670_REG_COM12, 0x78);
	regWrite(OV7670_REG_COM13, 0xC1);
	regWrite(OV7670_REG_COM14, 0x00);
	regWrite(OV7670_REG_EDGE, 0x00);
  
  // RGB565格式
  regWrite(OV7670_REG_COM15, 0xD0);
  regWrite(OV7670_REG_COM16, 0x08);
  regWrite(OV7670_REG_COM17, 0x80);

  regWrite(OV7670_REG_REG4B, 0x09);
  regWrite(OV7670_REG_DNSTH, 0x00);

  // 色彩矩阵系数
	regWrite(0x4F, 0x80);
	regWrite(0x50, 0x80);
	regWrite(0x51, 0x00);
	regWrite(0x52, 0x22);
	regWrite(0x53, 0x5E);
	regWrite(0x54, 0x80);

	regWrite(0x56, 0x40);

	regWrite(0x58, 0x9E);

  regWrite(0x59, 0x88);
  regWrite(0x64, 0x04);

	regWrite(0x65, 0x20);
	regWrite(0x66, 0x05);
	regWrite(0x69, 0x00);
	regWrite(0x6a, 0x40);
	regWrite(0x6b, 0x4a);
	regWrite(0x6c, 0x0a);
	regWrite(0x6d, 0x55);
	regWrite(0x6e, 0x11);
	regWrite(0x6f, 0x9f); 

  // 开启测试模式
	// regWrite(0x70, 0xFF);
	// regWrite(0x71, 0xFF);
	regWrite(0x70, 0x3a);
	regWrite(0x71, 0x35); 

	regWrite(0x71, 0x11);
	regWrite(0x72, 0x00);
	regWrite(0x73, 0x00);
	regWrite(0x74, 0x19);
	regWrite(0x75, 0x05);
	regWrite(0x76, 0xe1);
	regWrite(0x77, 0x01);
	regWrite(0x7A, 0x20);
	regWrite(0x7B, 0x1c); 


  regWrite(0x7C, 0x28);
  regWrite(0x7D, 0x3c);
  regWrite(0x7E, 0x1c);
  regWrite(0x7F, 0x68);

  regWrite(0x80, 0x76);
  regWrite(0x81, 0x80);
  regWrite(0x82, 0x88);
  regWrite(0x83, 0x8f);
  regWrite(0x84, 0x96);
  regWrite(0x85, 0xa3);
  regWrite(0x86, 0xaf);
  regWrite(0x87, 0xc4);
  regWrite(0x88, 0xd7);
  regWrite(0x89, 0xe8);
  regWrite(0x8C, 0x00);

  regWrite(0x92, 0x00);
  regWrite(0x94, 0x04);
  regWrite(0x95, 0x08);

  regWrite(0x9D, 0x4c);
  regWrite(0x9E, 0x3f);
  regWrite(0x9F, 0x78);

  regWrite(0xA0, 0x68);
  regWrite(0xA2, 0x00);
  regWrite(0xA4, 0x89);
  regWrite(0xA5, 0x05);
  regWrite(0xA6, 0xdf);
  regWrite(0xA7, 0xdf);
  regWrite(0xA8, 0xf0);
  regWrite(0xA9, 0x90);

  regWrite(0xAA, 0x94);
  regWrite(0xAB, 0x07);

  regWrite(0xB1, 0x0c);
  regWrite(0xB3, 0x82);
  
  regWrite(0xC9, 0x60);

  printf("OV7670 init done\n");

  /********************* 启动 DCMI + DMA *********************/
  // 关闭所有DCMI中断，只开帧完成中断
  __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC | DCMI_IT_ERR | DCMI_IT_OVR);

  // CAPCNT = 时钟数-1, RGB565 8-bit总线每像素2时钟: 128px*2-1=255, 必须是4的倍数-1(255=256-1 ✓)
  // X0=192: (320-128)/2=96px * 2clk = 192时钟偏移
  // VLINE = 行数-1: 160-1=159
  HAL_DCMI_ConfigCrop(&hdcmi, 192, 40, 255, 159);
  HAL_DCMI_EnableCrop(&hdcmi);

  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame, OV7670_FRAME_WORD_BUFF);
}

void OV7670_Snapshot(void)
{
  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame, OV7670_FRAME_WORD_BUFF);
}

uint32_t* OV7670_GetFrameBuffer(void)
{
  return frame;
}

void OV7670_Test(void)
{

}

void OV7670_Write(void)
{

}
