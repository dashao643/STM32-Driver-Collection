#ifndef __MAX30102_H__
#define __MAX30102_H__

// INT: 中断触发引脚,下降沿触发

// #define MAX30102_INSTANCE                 HI2C1
#define MAX30102_HANDLE                   &hi2c1
#define MAX30102_TX_TIMEOUT_MS            100

#define MAX30102_SLAVE_ADDRESS            0xAE

void MAX30102_Init(void);
void MAX30102_ClearIRQ(void);

void MAX30102_Write_Test(void);
void MAX30102_Read_Test(void);

void readFIFO(void);

#endif
