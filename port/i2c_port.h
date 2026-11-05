/*
 * i2c_port.h - hardware abstraction layer used by the MPU6050 driver.
 *
 * The driver never calls STM32/ESP functions directly. It only calls the
 * three functions below. Each platform (STM32, ESP32, PC test) provides
 * its own .c file that implements them. This keeps the driver portable
 * and lets us unit-test it on a PC with a fake (mock) I2C bus.
 */
#ifndef I2C_PORT_H
#define I2C_PORT_H

#include <stdint.h>

/* Return value: 0 = success, non-zero = bus error */
int  port_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value);
int  port_i2c_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint16_t len);
void port_delay_ms(uint32_t ms);

#endif /* I2C_PORT_H */
