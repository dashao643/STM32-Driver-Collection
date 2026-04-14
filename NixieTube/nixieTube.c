#include "nixieTube.h"

static NixieTube_HandleTypeDef nixieTube = {0};

// 关闭所有位选
static void closeAllDigit(void)
{
  DIG1_OFF();
  DIG2_OFF();
  DIG3_OFF();
  DIG4_OFF();
}

// 关闭所有位段
static void closeAllSegment(void)
{
  NIXIE_TUBE_A_OFF();
  NIXIE_TUBE_B_OFF();
  NIXIE_TUBE_C_OFF();
  NIXIE_TUBE_D_OFF();
  NIXIE_TUBE_E_OFF();
  NIXIE_TUBE_F_OFF();
  NIXIE_TUBE_G_OFF();
  NIXIE_TUBE_DP_OFF();
}

static void openSegment(uint8_t num)
{
  closeAllSegment();

  switch(num)
  {
    case 0:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();
      NIXIE_TUBE_D_ON();NIXIE_TUBE_E_ON();NIXIE_TUBE_F_ON();
      break;
    case 1:
      NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();
      break;
    case 2:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_B_ON();NIXIE_TUBE_G_ON();
      NIXIE_TUBE_E_ON();NIXIE_TUBE_D_ON();
      break;
    case 3:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();
      NIXIE_TUBE_D_ON();NIXIE_TUBE_G_ON();
      break;
    case 4:
      NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();NIXIE_TUBE_F_ON();
      NIXIE_TUBE_G_ON();
      break;
    case 5:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_C_ON();NIXIE_TUBE_D_ON();
      NIXIE_TUBE_F_ON();NIXIE_TUBE_G_ON();
      break;
    case 6:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_C_ON();NIXIE_TUBE_D_ON();
      NIXIE_TUBE_E_ON();NIXIE_TUBE_F_ON();NIXIE_TUBE_G_ON();
      break;
    case 7:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();
      break;
    case 8:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();
      NIXIE_TUBE_D_ON();NIXIE_TUBE_E_ON();NIXIE_TUBE_F_ON();
      NIXIE_TUBE_G_ON();
      break;
    case 9:
      NIXIE_TUBE_A_ON();NIXIE_TUBE_B_ON();NIXIE_TUBE_C_ON();
      NIXIE_TUBE_D_ON();NIXIE_TUBE_F_ON();NIXIE_TUBE_G_ON();
      break;
    default: break;
  }
}

// 关闭所有位选（消隐），设置段码，开启对应位显示数字

static void openDigitNum(uint8_t num)
{
  closeAllDigit();

  openSegment(num);

  switch(nixieTube.digX){
    case DIGIT_1: DIG1_ON(); break;
    case DIGIT_2: DIG2_ON(); break;
    case DIGIT_3: DIG3_ON(); break;
    case DIGIT_4: DIG4_ON(); break;
    default: return;
  }
}

static uint8_t byteToBcd2(uint8_t Value)
{
  uint32_t bcdhigh = 0U;

  while (Value >= 10U){
    bcdhigh++;
    Value -= 10U;
  }

  return ((uint8_t)(bcdhigh << 4U) | Value);
}

void NixieTube_Init(void)
{
  nixieTube.digX = 0;
  nixieTube.updateFlag = false;
}

/**
 * @brief 定时器1毫秒调用一次。digX = 1 ~ DIGIT_CNT 不断循环
 * 
 */

void NixieTube_SetUpdateFlag(void)
{
  nixieTube.updateFlag = true;
  nixieTube.digX = (nixieTube.digX % DIGIT_CNT) + 1;
}

/**
 * @brief 显示两个两位数,范围0-99
 * 
 */
void NixieTube_ShowTask2(uint8_t num1, uint8_t num2)
{
  if(!nixieTube.updateFlag)
    return;
  
  if(num1 > 99) num1 = 99;
  if(num2 > 99) num2 = 99;

  num1 = byteToBcd2(num1);
  num2 = byteToBcd2(num2);

  uint8_t num1L = num1 >> 4;
  uint8_t num1R = num1 & 0xF;

  uint8_t num2L = num2 >> 4;
  uint8_t num2R = num2 & 0xF;

  switch(nixieTube.digX){
  case DIGIT_1: openDigitNum(num1L); break;
  case DIGIT_2: openDigitNum(num1R); NIXIE_TUBE_DP_ON(); break;
  case DIGIT_3: openDigitNum(num2L); break;
  case DIGIT_4: openDigitNum(num2R); break;
  }

  nixieTube.updateFlag = false;
}

/**
 * @brief 显示一个四位数,范围0-9999
 * 
 */
void NixieTube_ShowTask1(uint16_t num)
{
  if(!nixieTube.updateFlag)
    return;

  if(num > 9999)
    num = 9999;

  uint8_t digit1 = (num / 1000) % 10;   // 千位
  uint8_t digit2 = (num / 100) % 10;    // 百位
  uint8_t digit3 = (num / 10) % 10;     // 十位
  uint8_t digit4 = num % 10;            // 个位

  switch(nixieTube.digX){
  case DIGIT_1: openDigitNum(digit1); break;
  case DIGIT_2: openDigitNum(digit2); break;
  case DIGIT_3: openDigitNum(digit3); break;
  case DIGIT_4: openDigitNum(digit4); break;
  }

  nixieTube.updateFlag = false;
}
