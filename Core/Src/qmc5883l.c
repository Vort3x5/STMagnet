#include "qmc5883l.h"
#include <stdio.h>

HAL_StatusTypeDef QMC5883P_Init(QMC5883P_t *dev, I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t config[2];
    uint8_t chip_id;
    
    dev->hi2c = hi2c;
    dev->x = 0;
    dev->y = 0;
    dev->z = 0;
    dev->initialized = 0;
    
    printf("QMC5883P: Checking device...\r\n");
    status = HAL_I2C_IsDeviceReady(dev->hi2c, QMC5883P_ADDR, 3, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: Device not ready!\r\n");
        return status;
    }
    printf("QMC5883P: Device ready\r\n");
    
    HAL_Delay(10);
    
    // Read and verify chip ID
    printf("QMC5883P: Reading chip ID...\r\n");
    config[0] = QMC5883P_REG_CHIP_ID;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 1, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: Failed to request chip ID\r\n");
        return status;
    }
    
    status = HAL_I2C_Master_Receive(dev->hi2c, QMC5883P_ADDR, &chip_id, 1, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: Failed to read chip ID\r\n");
        return status;
    }
    
    printf("QMC5883P: Chip ID = 0x%02X (expected 0x80)\r\n", chip_id);
    if (chip_id != QMC5883P_CHIP_ID) {
        printf("QMC5883P: Wrong chip ID!\r\n");
        return HAL_ERROR;
    }
    
    // Soft reset
    printf("QMC5883P: Soft reset...\r\n");
    config[0] = QMC5883P_REG_CONF2;
    config[1] = 0x80;  // Soft reset bit
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 2, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: Soft reset failed\r\n");
        return status;
    }
    HAL_Delay(50);
    
    // Unlock hidden registers
    printf("QMC5883P: Unlocking hidden registers...\r\n");
    config[0] = QMC5883P_REG_UNLOCK;
    config[1] = QMC5883P_UNLOCK_VALUE;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 2, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: Unlock failed\r\n");
        return status;
    }
    
    // Configure XYZ sign
    printf("QMC5883P: Configuring XYZ sign...\r\n");
    config[0] = QMC5883P_REG_XYZ_SIGN;
    config[1] = QMC5883P_XYZ_SIGN_VALUE;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 2, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: XYZ sign config failed\r\n");
        return status;
    }
    
    // Configure CONF1 (but not continuous mode yet)
    printf("QMC5883P: Configuring CONF1...\r\n");
    config[0] = QMC5883P_REG_CONF1;
    config[1] = 0x01;  // Standby mode first
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 2, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: CONF1 config failed\r\n");
        return status;
    }
    
    // Configure CONF2
    printf("QMC5883P: Configuring CONF2...\r\n");
    config[0] = QMC5883P_REG_CONF2;
    config[1] = 0x00;
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 2, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: CONF2 config failed\r\n");
        return status;
    }
    
    HAL_Delay(10);
    
    // Now set continuous mode with proper config
    printf("QMC5883P: Starting continuous mode...\r\n");
    config[0] = QMC5883P_REG_CONF1;
    config[1] = 0x8D;  // OSR=512(10), RNG=8G(00), ODR=100Hz(11), Mode=Cont(01)
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 2, 1000);
    if (status != HAL_OK) {
        printf("QMC5883P: Continuous mode failed\r\n");
        return status;
    }
    
    HAL_Delay(50);
    
    // Read back CONF1 to verify
    config[0] = QMC5883P_REG_CONF1;
    HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, config, 1, 1000);
    HAL_I2C_Master_Receive(dev->hi2c, QMC5883P_ADDR, &chip_id, 1, 1000);
    printf("QMC5883P: CONF1 readback = 0x%02X\r\n", chip_id);
    
    printf("QMC5883P: Initialization complete!\r\n");
    dev->initialized = 1;
    return HAL_OK;
}

HAL_StatusTypeDef QMC5883P_ReadRaw(QMC5883P_t *dev)
{
    HAL_StatusTypeDef status;
    uint8_t data[6];
    uint8_t reg = QMC5883P_REG_XOUT_L;
    
    status = HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, &reg, 1, 1000);
    if (status != HAL_OK) return status;
    
    status = HAL_I2C_Master_Receive(dev->hi2c, QMC5883P_ADDR, data, 6, 1000);
    if (status != HAL_OK) return status;
    
    dev->x = (int16_t)(data[1] << 8 | data[0]);
    dev->y = (int16_t)(data[3] << 8 | data[2]);
    dev->z = (int16_t)(data[5] << 8 | data[4]);
    
    return HAL_OK;
}

uint8_t QMC5883P_IsDataReady(QMC5883P_t *dev)
{
    uint8_t status;
    uint8_t reg = QMC5883P_REG_STATUS;
    
    HAL_I2C_Master_Transmit(dev->hi2c, QMC5883P_ADDR, &reg, 1, 1000);
    HAL_I2C_Master_Receive(dev->hi2c, QMC5883P_ADDR, &status, 1, 1000);
    
    return (status & 0x01);
}
