#include "stm32f4xx_hal.h"
#include "max30102.h"
#include "max30102_def.h"
#include "i2c.h"

#include <stdint.h>
#include <stdio.h>

static void memRead(uint16_t memAddr, uint8_t *data)
{
  HAL_I2C_Mem_Read(MAX30102_HANDLE, MAX30102_SLAVE_ADDRESS,
    memAddr, 1, data, 1, MAX30102_TX_TIMEOUT_MS);
}

static void memWrite(uint16_t memAddr, uint8_t data)
{
  HAL_I2C_Mem_Write(MAX30102_HANDLE, MAX30102_SLAVE_ADDRESS,
    memAddr, 1, &data, 1, MAX30102_TX_TIMEOUT_MS);
}

void MAX30102_Init(void)
{
  // 轮询方法,关闭所有中断
  memWrite(MAX30102_INTERRUPT_ENABLE_1, 0x00);
  memWrite(MAX30102_INTERRUPT_ENABLE_2, 0x00);

  // 清除读写指针
  memWrite(MAX30102_FIFO_WRITE_POINTER, 0x00);
  memWrite(MAX30102_OVERFLOW_COUNTER, 0x00);
  memWrite(MAX30102_FIFO_READ_POINTER, 0x00);

  // SMP_AVE=1, rollover=0, almost full=17
  memWrite(MAX30102_FIFO_CONFIGURATION, 0x0F);
  // SpO2 mode: Red + IR(计算心率+血氧)
  memWrite(MAX30102_MODE_CONFIGURATION, 0x03);
  // 4096nA, 100Hz, 411us (18bit)
  memWrite(MAX30102_SPO2_CONFIGURATION, 0x27);
  // LED1 (Red) ~7mA
  // memWrite(MAX30102_LED_PULSE_AMPLITUDE_1, 0x24);
  memWrite(MAX30102_LED_PULSE_AMPLITUDE_1, 0x40);
  // LED2 (IR) ~7mA
  // memWrite(MAX30102_LED_PULSE_AMPLITUDE_2, 0x24);
  memWrite(MAX30102_LED_PULSE_AMPLITUDE_2, 0x40);
  // Pilot LED ~25mA
  memWrite(MAX30102_PROXIMITY_MODE_LED_PULSE_AMPLITUDE, 0x7F);
  // memWrite(MAX30102_PROXIMITY_MODE_LED_PULSE_AMPLITUDE, 0xFF);
}

/**
 * @brief 从FIFO读取指定数量的红光和红外数据
 * 
 * @param samples 红光和红外数据结构体数组
 * @param maxSamples 最大读取数量
 * @return uint8_t 实际读取数量
 */
uint8_t MAX30102_ReadFifo(MAX30102_Sample_t *samples, uint8_t maxSamples)
{
  uint8_t writePtr = 0;     // FIFO写指针位置
  uint8_t readPtr = 0;      // FIFO读指针位置
  uint8_t available = 0;    // FIFO可读取的数据量
  uint8_t resCount = 0;     // 返回实际读取数量
  uint8_t fifoData[6];      // FIFO 6 字节数据

  memRead(MAX30102_FIFO_WRITE_POINTER, &writePtr);
  memRead(MAX30102_FIFO_READ_POINTER, &readPtr);

  // 只取后五位
  writePtr &= 0x1F;
  readPtr  &= 0x1F;
  available = (writePtr - readPtr) & 0x1F;

  if (available == 0 || maxSamples == 0 || samples == NULL)
    return 0;

  resCount = (available < maxSamples) ? available : maxSamples;

  for (uint8_t i = 0; i < resCount; i++) {
    HAL_I2C_Mem_Read(MAX30102_HANDLE, MAX30102_SLAVE_ADDRESS,
      MAX30102_FIFO_DATA_REGISTER, 1, fifoData, 6, MAX30102_TX_TIMEOUT_MS);

    samples[i].red = ((uint32_t)fifoData[0] << 16) |
                     ((uint32_t)fifoData[1] << 8) |
                     (uint32_t)fifoData[2];
    samples[i].ir  = ((uint32_t)fifoData[3] << 16) |
                     ((uint32_t)fifoData[4] << 8) |
                     (uint32_t)fifoData[5];
    // red和ir18位数据 
    samples[i].red &= 0x3FFFF;
    samples[i].ir  &= 0x3FFFF;
  }

  return resCount;
}

void MAX30102_ClearIRQ(void)
{
  uint8_t data = 0;
  memRead(MAX30102_INTERRUPT_STATUS_1, &data);
  memRead(MAX30102_INTERRUPT_STATUS_2, &data);
}
