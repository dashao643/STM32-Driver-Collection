#include "max30102_app.h"
#include "max30102.h"
#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PRINT_DEBUG

// #define MAX30102_ALGO_BUFFER_SIZE      500
#define FINGER_ON_THRESHOLD     60000          // 检测到手指下阈值(IR大于此值,检测到手指)
#define FINGER_OFF_THRESHOLD    40000          // 无手指上阈值(IR小于此值,未检测到手指)
#define FINGER_CONFIRM_COUNT    5              // 是否检测到手指的确认次数

// 主循环状态机
typedef enum {
  STATE_IDLE,         // 空闲状态
  STATE_COLLECTING,   // 采集状态
  STATE_CALCULATING,  // 计算状态
} DetectState_e;

static uint32_t irBuf[MAX30102_ALGO_BUFFER_SIZE];
static uint32_t redBuf[MAX30102_ALGO_BUFFER_SIZE];
static uint16_t sampleIdx;                  // 样本缓冲区索引
static DetectState_e state;
static MAX30102_AlgoOutput_t lastResult;
static uint8_t fingerConfirmCnt;            // 检测到手指
static uint8_t offConfirmCnt;               // 未检测到手指
static uint32_t timer;

void MAX30102_APP_Init(void)
{
  state = STATE_IDLE;
  sampleIdx = 0;
  fingerConfirmCnt = 0;
  offConfirmCnt = 0;
  memset(&lastResult, 0, sizeof(lastResult));

  timer = HAL_GetTick();

  MAX30102_Init();
}

// 应用层轮询任务
void MAX30102_APP_Task(void)
{
  if(HAL_GetTick() - timer < MAX30102_APP_READ_INTERVAL_MS)
    return;

  timer = HAL_GetTick();

  MAX30102_Sample_t samples[16];
  uint8_t count = MAX30102_ReadFifo(samples, 16);

  // printf("count=%d\n",count);

  if(count == 0) return;

  for (uint8_t i = 0; i < count; i++) {
    uint32_t ir = samples[i].ir;
    uint32_t red = samples[i].red;

    // printf("ir=%d\n",ir);
    // printf("red=%d\n",red);

    switch (state) {
      case STATE_IDLE:
        if (ir > FINGER_ON_THRESHOLD) {
          fingerConfirmCnt++;
          if (fingerConfirmCnt >= FINGER_CONFIRM_COUNT) {
            fingerConfirmCnt = 0;
            sampleIdx = 0;
            state = STATE_COLLECTING;
            printf("[MAX30102] Finger detected, start collecting...\n");
          }
        } else {
          fingerConfirmCnt = 0;
        }
        break;

      case STATE_COLLECTING:
        if (ir < FINGER_OFF_THRESHOLD) {
          offConfirmCnt++;
          if (offConfirmCnt >= FINGER_CONFIRM_COUNT) {
            offConfirmCnt = 0;
            sampleIdx = 0;
            state = STATE_IDLE;
            printf("[MAX30102] Finger removed during collection, reset.\n");
            break;
          }
        } else {
          offConfirmCnt = 0;
        }

        if (sampleIdx < MAX30102_ALGO_BUFFER_SIZE) {
          irBuf[sampleIdx] = ir;
          redBuf[sampleIdx] = red;
          sampleIdx++;
        }

        if (sampleIdx >= MAX30102_ALGO_BUFFER_SIZE) {
          state = STATE_CALCULATING;
        }
        break;

      case STATE_CALCULATING: {
        MAX30102_AlgoInput_t input = {
          .irBuffer = irBuf,
          .redBuffer = redBuf,
          .length = MAX30102_ALGO_BUFFER_SIZE
        };
        MAX30102_Algo_Process(&input, &lastResult);
        state = STATE_COLLECTING;   // 连续检测
        sampleIdx = 0;

        if (lastResult.hrValid) {
          printf("[MAX30102] Heart Rate: %ld BPM\n", lastResult.heartRate);
        } else {
          printf("[MAX30102] Heart Rate: invalid\n");
        }
        if (lastResult.spo2Valid) {
          printf("[MAX30102] SpO2: %ld %%\n", lastResult.spo2);
        } else {
          printf("[MAX30102] SpO2: invalid\n");
        }
        break;
      }

      // case STATE_RESULT:
      //   if (ir < FINGER_OFF_THRESHOLD) {
      //     offConfirmCnt++;
      //     if (offConfirmCnt >= FINGER_CONFIRM_COUNT) {
      //       offConfirmCnt = 0;
      //       sampleIdx = 0;
      //       state = STATE_IDLE;
      //       printf("[MAX30102] Finger removed, reset.\n");
      //     }
      //   } else {
      //     offConfirmCnt = 0;
      //   }
      //   break;
    }
  }
}

// 获取最近一次计算结果
void MAX30102_APP_GetResult(MAX30102_AlgoOutput_t *resValue)
{
  if(resValue == NULL) return;

  *resValue = lastResult;

  // if (heartRate != NULL) *heartRate = lastResult.heartRate;
  // if (spo2 != NULL)      *spo2 = lastResult.spo2;
  // if (hrValid != NULL)   *hrValid = lastResult.hrValid;
  // if (spo2Valid != NULL) *spo2Valid = lastResult.spo2Valid;
}

// 获取当前采集进度 (0~100)
uint8_t MAX30102_APP_GetProgress(void)
{
  if (state == STATE_IDLE) return 0;
    
  return (uint8_t)((sampleIdx * 100U) / MAX30102_ALGO_BUFFER_SIZE);
}
