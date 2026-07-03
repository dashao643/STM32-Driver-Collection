#ifndef __MAX30102_APP_H__
#define __MAX30102_APP_H__

#include "max30102_al.h"

#include <stdint.h>
#include <stdbool.h>

// 读数不准!

#define MAX30102_APP_READ_INTERVAL_MS        100       // 20~50ms

void MAX30102_APP_Init(void);
void MAX30102_APP_Task(void);
void MAX30102_APP_GetResult(MAX30102_AlgoOutput_t *resValue);
uint8_t MAX30102_APP_GetProgress(void);

#endif
