#ifndef QMC5883L_H
#define QMC5883L_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define QMC5883L_ADDR       (0x0D << 1)

typedef enum 
{
    XOUT_L = 0x0,
    XOUT_H,
    YOUT_L,
    YOUT_H,
    ZOUT_L,
    ZOUT_H,
    STATUS,
    TOUT_L,
    TOUT_H,
    CONTROL1,
    CONTROL2,
    PERIOD,
} QMC5883L_Reg;

/* Control Register 1 bit definitions */
#define QMC5883L_MODE_STANDBY   0x00
#define QMC5883L_MODE_CONTINUOUS 0x01

#define QMC5883L_ODR_10HZ       0x00
#define QMC5883L_ODR_50HZ       0x04
#define QMC5883L_ODR_100HZ      0x08
#define QMC5883L_ODR_200HZ      0x0C

#define QMC5883L_RNG_2G         0x00
#define QMC5883L_RNG_8G         0x10

#define QMC5883L_OSR_512        0x00
#define QMC5883L_OSR_256        0x40
#define QMC5883L_OSR_128        0x80
#define QMC5883L_OSR_64         0xC0

typedef struct 
{
    I2C_HandleTypeDef *hi2c;
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t initialized;
} QMC5883L_t;

HAL_StatusTypeDef QMC5883L_Init(QMC5883L_t *dev, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef QMC5883L_ReadRaw(QMC5883L_t *dev);
HAL_StatusTypeDef QMC5883L_ReadTemperature(QMC5883L_t *dev, int16_t *temp);
uint8_t QMC5883L_IsDataReady(QMC5883L_t *dev);

#endif
