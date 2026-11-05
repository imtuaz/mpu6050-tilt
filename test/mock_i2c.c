/*
 * mock_i2c.c - fake I2C bus for PC unit tests.
 *
 * Instead of a real sensor, we keep an array of 256 "registers".
 * Tests fill this array with chosen values, then call the driver and
 * check that it decodes them correctly.
 */
#include <string.h>
#include "i2c_port.h"
#include "mock_i2c.h"

uint8_t mock_regs[256];
int     mock_fail_bus = 0;   /* set to 1 to simulate a bus error */

void mock_reset(void)
{
    memset(mock_regs, 0, sizeof mock_regs);
    mock_regs[0x75] = 0x68;  /* WHO_AM_I default value */
    mock_regs[0x6B] = 0x40;  /* PWR_MGMT_1 reset value: sleep bit set */
    mock_fail_bus   = 0;
}

int port_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    (void)dev_addr;
    if (mock_fail_bus)
        return -1;
    mock_regs[reg] = value;
    return 0;
}

int port_i2c_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    (void)dev_addr;
    if (mock_fail_bus)
        return -1;
    for (uint16_t i = 0; i < len; i++)
        buf[i] = mock_regs[(uint8_t)(reg + i)];
    return 0;
}

void port_delay_ms(uint32_t ms)
{
    (void)ms;  /* no need to actually wait in tests */
}

/* Helper: store an int16 big-endian, the way the MPU6050 does */
void mock_set16(uint8_t reg, int16_t v)
{
    mock_regs[reg]                = (uint8_t)((uint16_t)v >> 8);
    mock_regs[(uint8_t)(reg + 1)] = (uint8_t)((uint16_t)v & 0xFF);
}
