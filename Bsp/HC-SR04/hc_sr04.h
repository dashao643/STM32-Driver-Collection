#ifndef __HC_SR04_H__
#define __HC_SR04_H__

// 电源: 5V

// TRIG初始推挽输出, 低电平
// ECHO初始下拉输入

#define HC_SR04_TIMEOUT_US        10000
#define HC_SR04_TRIGGER_US        20
#define HC_SR04_CAL_DISTANCE      (5.8)

void HC_SR04_Init(void);
HAL_StatusTypeDef HC_SR04_Measure(uint16_t *distance_mm);

#endif
