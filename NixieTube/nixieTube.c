#include "nixieTube.h"
#include <stdbool.h>

// 关闭所有位
static void closeAllBit(void)
{
  DIG1_OFF();
  DIG2_OFF();
  DIG3_OFF();
  DIG4_OFF();
}

// 清空所有段（全部熄灭）
static void clearAll(void)
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

static void displayNum(uint8_t num)
{
  clearAll();

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

// 先关所有位（消隐）
// 先设置段码
// 再开启对应位

// 显示某一位
static void displayBit(uint8_t bit, uint8_t num)
{
  closeAllBit();

  displayNum(num);

  switch(bit){
    case 1: DIG1_ON(); break;
    case 2: DIG2_ON(); break;
    case 3: DIG3_ON(); break;
    case 4: DIG4_ON(); break;
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

/**
 * @brief 显示两个两位数,范围0-99
 * 
 */
void NixieTube_ShowTask2(uint8_t num1, uint8_t num2)
{
  if(num1 > 99) num1 = 99;
  if(num2 > 99) num2 = 99;
  num1 = byteToBcd2(num1);
  num2 = byteToBcd2(num2);

  uint8_t num1L = num1 >> 4;
  uint8_t num1R = num1 & 0xF;

  uint8_t num2L = num2 >> 4;
  uint8_t num2R = num2 & 0xF;

  displayBit(1,num1L);
  HAL_Delay(1);
  displayBit(2,num1R);
  HAL_Delay(1);
  displayBit(3,num2L);
  HAL_Delay(1);
  displayBit(4,num2R);
  HAL_Delay(1);
}

/**
 * @brief 显示一个四位数,范围0-9999
 * 
 */
void NixieTube_ShowTask1(uint16_t num)
{
  if(num > 9999)
    num = 9999;
}
