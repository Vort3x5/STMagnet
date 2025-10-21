#ifndef QMC5883P_H
#define QMC5883P_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define QMC5883P_ADDR       (0x2C << 1)

// Register addresses for QMC5883P (different from QMC5883L!)
#define QMC5883P_REG_CHIP_ID    0x00
#define QMC5883P_REG_XOUT_L     0x01
#define QMC5883P_REG_XOUT_H     0x02
#define QMC5883P_REG_YOUT_L     0x03
#define QMC5883P_REG_YOUT_H     0x04
#define QMC5883P_REG_ZOUT_L     0x05
#define QMC5883P_REG_ZOUT_H     0x06
#define QMC5883P_REG_STATUS     0x09
#define QMC5883P_REG_CONF1      0x0A
#define QMC5883P_REG_CONF2      0x0B
#define QMC5883P_REG_UNLOCK     0x0D
#define QMC5883P_REG_XYZ_SIGN   0x29

// Expected chip ID
#define QMC5883P_CHIP_ID        0x80

// Configuration values
#define QMC5883P_MODE_CONTINUOUS 0x09
#define QMC5883P_MODE_STANDBY    0x08
#define QMC5883P_CONF1_INIT      0xCF  // OSR=64, Range=8G, ODR=200Hz
#define QMC5883P_CONF2_INIT      0x00
#define QMC5883P_UNLOCK_VALUE    0x40
#define QMC5883P_XYZ_SIGN_VALUE  0x06

typedef struct 
{
    I2C_HandleTypeDef *hi2c;
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t initialized;
} QMC5883P_t;

HAL_StatusTypeDef QMC5883P_Init(QMC5883P_t *dev, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef QMC5883P_ReadRaw(QMC5883P_t *dev);
uint8_t QMC5883P_IsDataReady(QMC5883P_t *dev);

#endif
