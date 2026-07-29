#ifndef __OV7670_H__
#define __OV7670_H__

#include <stdint.h>
#include <stdbool.h>

/*
object to ov7670
VSYNC 帧的开始/结束(输出信号)
HREF/HSYNC 行的开始/结束(输出信号)
PCLK:像素时钟(输出信号)       PCL上升沿时读取数据
XCLK:系统时钟(输入信号)       MCU提供给ov7670信号
D0-D7:数据端口(输出信号)
RESTE:复位端口(正常使用拉高)  低电平复位
PWDN:功耗选择模式(正常使用拉低)

XCLK 接TIM1/8/9-11:168MHZ, PSC=0, ARR=7, CCR=4, 50%占空比PWM, 时钟频率=21MHZ(10-48)
DCMI Mode:                                    DCMI Slave 8 bits External Synchro
在该时钟边沿时，DCMI会对数据线上的信号进行采样
DCMI Mode Config:
Pixel clock polarity                          Active on Rising edge
Vertical synchronization polarity             Active High 
Horizontal synchronization polarity           Active Low
Frequency Of frame capture                    AII frames are captured
JPEG mode                                     Disabled

DMA Mode:                                     Circular
lncrement Address:                            ✔Memory
*/

#define OV7670_SLAVE_ADDR               0X42

#define OV7670_PID_VAL                  0x76
#define OV7670_VER_VAL                  0x73

#define OV7670_FRAME_WORD_BUFF          10240      // 存储一帧数据大小的数组 128 * 160 * 2 / 4 (40960 Byte = 10240 Word)

// #define OV7670_FRAME_WORD_BUFF          1000  // 进五次中断
// #define OV7670_FRAME_WORD_BUFF          5000  // 进1次中断
// #define OV7670_FRAME_WORD_BUFF          10000  // 开启两次进1次中断!!!

void OV7670_Init(void);
void OV7670_Snapshot(void);
uint32_t* OV7670_GetFrameBuffer(void);

void OV7670_Test(void);
void OV7670_Write(void);

#endif
