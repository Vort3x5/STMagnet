#include "qmc5883l.h"

HAL_StatusTypeDef QMC5883L_Init(QMC5883L_t *dev, I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t config[2];
    
    dev->hi2c = hi2c;
    dev->x = 0;
    dev->y = 0;
    dev->z = 0;
    dev->initialized = 0;

	HAL_I2C_DeInit(dev->hi2c);
	HAL_Delay(10);
	HAL_I2C_Init(dev->hi2c);
	HAL_Delay(10);
    
    config[0] = CONTROL2;
    config[1] = 0x80;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883L_ADDR, config, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    HAL_Delay(10);
    
    config[0] = CONTROL1;
    config[1] = QMC5883L_MODE_CONTINUOUS | 
                QMC5883L_ODR_100HZ | 
                QMC5883L_RNG_2G | 
                QMC5883L_OSR_512;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883L_ADDR, config, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    config[0] = PERIOD;
    config[1] = 0x01;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883L_ADDR, config, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    HAL_Delay(10);
    
    dev->initialized = 1;
    return HAL_OK;
}

HAL_StatusTypeDef QMC5883L_ReadRaw(QMC5883L_t *dev)
{
    HAL_StatusTypeDef status;
    uint8_t data[6];
    uint8_t reg = XOUT_L;
    
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883L_ADDR, &reg, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    status = HAL_I2C_Master_Receive(dev->hi2c, QMC5883L_ADDR, data, 6, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    dev->x = (int16_t)(data[1] << 8 | data[0]);
    dev->y = (int16_t)(data[3] << 8 | data[2]);
    dev->z = (int16_t)(data[5] << 8 | data[4]);
    
    return HAL_OK;
}

uint8_t QMC5883L_IsDataReady(QMC5883L_t *dev)
{
    uint8_t status;
    uint8_t reg = STATUS;
    
    HAL_I2C_Master_Transmit(dev->hi2c, QMC5883L_ADDR, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(dev->hi2c, QMC5883L_ADDR, &status, 1, HAL_MAX_DELAY);
    
    return (status & 0x01);
}

HAL_StatusTypeDef QMC5883L_ReadTemperature(QMC5883L_t *dev, int16_t *temp)
{
    HAL_StatusTypeDef status;
    uint8_t data[2];
    uint8_t reg = TOUT_L;
    
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883L_ADDR, &reg, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    status = HAL_I2C_Master_Receive(dev->hi2c, QMC5883L_ADDR, data, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) 
		return status;
    
    *temp = (int16_t)(data[1] << 8 | data[0]);
    
    return HAL_OK;
}
