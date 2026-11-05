/*
 * i2c_port_stm32.c - i2c_port implementation for STM32 HAL.
 *
 * Assumes CubeMX generated an I2C handle named hi2c1 (I2C1, 100 or 400 kHz).
 * Add this file and the lib/ folder to your STM32CubeIDE project.
 */
#include "i2c_port.h"
#include "main.h"   /* CubeMX header: pulls in stm32xxxx_hal.h */

extern I2C_HandleTypeDef hi2c1;

#define I2C_TIMEOUT_MS 50

int port_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    /* HAL expects the 7-bit address shifted left by 1 */
    HAL_StatusTypeDef st = HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(dev_addr << 1),
                                             reg, I2C_MEMADD_SIZE_8BIT,
                                             &value, 1, I2C_TIMEOUT_MS);
    return (st == HAL_OK) ? 0 : -1;
}

int port_i2c_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(dev_addr << 1),
                                            reg, I2C_MEMADD_SIZE_8BIT,
                                            buf, len, I2C_TIMEOUT_MS);
    return (st == HAL_OK) ? 0 : -1;
}

void port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
