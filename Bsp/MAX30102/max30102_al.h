#ifndef __MAX30102_AL_H__
#define __MAX30102_AL_H__

#include <stdint.h>
#include <stdbool.h>

// MAX30102算法函数接口

#define MAX30102_ALGO_BUFFER_SIZE  500

typedef struct {
  uint32_t *irBuffer;
  uint32_t *redBuffer;
  uint16_t length;
} MAX30102_AlgoInput_t;

typedef struct {
  int32_t heartRate;   // 心率, 单位: BPM
  int32_t spo2;        // 血氧饱和度, 单位: %
  bool hrValid;        // 表示心率值有效性
  bool spo2Valid;      // 表示血氧值有效性
} MAX30102_AlgoOutput_t;

void MAX30102_Algo_Process(const MAX30102_AlgoInput_t *input, MAX30102_AlgoOutput_t *output);

#endif
