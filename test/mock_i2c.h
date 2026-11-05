#ifndef MOCK_I2C_H
#define MOCK_I2C_H

#include <stdint.h>

extern uint8_t mock_regs[256];
extern int     mock_fail_bus;

void mock_reset(void);
void mock_set16(uint8_t reg, int16_t v);

#endif /* MOCK_I2C_H */
