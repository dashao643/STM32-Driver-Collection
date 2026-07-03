#ifndef __MAX30102_H__
#define __MAX30102_H__

#include <stdint.h>

#define MAX30102_HANDLE                   &hi2c1
#define MAX30102_TX_TIMEOUT_MS            100
#define MAX30102_SLAVE_ADDRESS            0xAE

typedef struct {
  uint32_t red;
  uint32_t ir;
} MAX30102_Sample_t;

void MAX30102_Init(void);
uint8_t MAX30102_ReadFifo(MAX30102_Sample_t *samples, uint8_t maxSamples);

void MAX30102_ClearIRQ(void);

#endif
