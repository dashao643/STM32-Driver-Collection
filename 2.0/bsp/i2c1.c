#include "stm32f1xx_hal.h"
#include "i2c1.h"
#include "general.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// #define SCL_PB6_SDA_PB7
// #define SCL_PB8_SDA_PB9

// PB8 ------> SCL
// PB9 ------> SDA

#define I2C_TIMOUT_MS           200

static I2C_HandleTypeDef i2c1;

void I2C1_Init(void)
{
    i2c1.Instance = I2C1;
    i2c1.Init.ClockSpeed = 400000;          // 高速模式
    i2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    i2c1.Init.OwnAddress1 = 0;
    i2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c1.Init.OwnAddress2 = 0;
    i2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&i2c1);
}

void I2C1_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef gpio = {0};

    if(hi2c->Instance == I2C1) {
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /********************* PB8 SCL *********************/
        /********************* PB9 SDA *********************/
        gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        gpio.Mode = GPIO_MODE_AF_OD;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        
        HAL_GPIO_Init(GPIOB, &gpio);

// #if defined SCL_PB6_SDA_PB7
        // __HAL_AFIO_REMAP_I2C1_DISABLE();
// #elif defined SCL_PB8_SDA_PB9
        __HAL_AFIO_REMAP_I2C1_ENABLE();
// #endif
        __HAL_RCC_I2C1_CLK_ENABLE();
    }
}

HAL_StatusTypeDef I2C1_Mem_Write(uint16_t devAddress, uint16_t memAddress, 
    uint16_t memAddSize, const uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Write(&i2c1, devAddress, memAddress,  memAddSize, (uint8_t*)data, size, I2C_TIMOUT_MS);
}

HAL_StatusTypeDef I2C1_Mem_Read(uint16_t devAddress, uint16_t memAddress, 
    uint16_t memAddSize, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Read(&i2c1, devAddress, memAddress,  memAddSize, data, size, I2C_TIMOUT_MS);
}

// HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, 
//     uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
// HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, 
//     uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);