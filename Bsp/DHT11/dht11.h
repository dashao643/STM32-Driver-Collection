#ifndef __DHT11_H__
#define __DHT11_H__

#include "main.h"
#include <stdint.h>

// SDA配置为开漏输出,上拉 
// 每次通信读出的温湿度数值为上一次通信时DHT11采集的温湿度数据,故建议使用时隔2s连续2次读取DHT11传感器

#define DHT11_MIN_INTERVAL_MS       2000      // 最小读取间隔

#define DHT11_START_LOW_MS          25        // 起始信号拉低时长18-35毫秒
// #define DHT11_RESPONSE_TIMEOUT_US   100       // 等待从机回应超时时间

typedef struct {
  uint8_t dataArr[5];    // 接收5字节数据: 湿度 湿度小数 温度 温度小数 校验和
  uint32_t timer;
} DHT11_t;

void DHT11_Init(void);
void DHT11_Read(uint8_t *temp, uint8_t *humi);

#endif