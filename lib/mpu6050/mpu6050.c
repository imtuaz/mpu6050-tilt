#include "mpu6050.h"
#include "i2c_port.h"

/* Combine two bytes (high first, as the MPU6050 sends them) into int16 */
static int16_t be16(const uint8_t *p)
{
    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int mpu6050_init(void)
{
    uint8_t id = 0;

    /* 1. Check that the right chip answers */
    if (port_i2c_read_regs(MPU6050_ADDR, MPU6050_REG_WHO_AM_I, &id, 1) != 0)
        return MPU6050_ERR_BUS;
    if (id != MPU6050_WHO_AM_I_VAL)
        return MPU6050_ERR_ID;

    /* 2. Wake up: the chip starts in sleep mode (PWR_MGMT_1 = 0x40) */
    if (port_i2c_write_reg(MPU6050_ADDR, MPU6050_REG_PWR_MGMT1, 0x00) != 0)
        return MPU6050_ERR_BUS;
    port_delay_ms(100);

    /* 3. Full-scale ranges: accel +-2 g, gyro +-250 dps (both = 0x00) */
    if (port_i2c_write_reg(MPU6050_ADDR, MPU6050_REG_ACCEL_CFG, 0x00) != 0)
        return MPU6050_ERR_BUS;
    if (port_i2c_write_reg(MPU6050_ADDR, MPU6050_REG_GYRO_CFG, 0x00) != 0)
        return MPU6050_ERR_BUS;

    return MPU6050_OK;
}

int mpu6050_read_raw(mpu6050_raw_t *raw)
{
    uint8_t buf[14];

    /* Burst read 14 bytes starting at ACCEL_XOUT_H:
     * [0..5] accel X,Y,Z  [6..7] temp  [8..13] gyro X,Y,Z  (each H,L) */
    if (port_i2c_read_regs(MPU6050_ADDR, MPU6050_REG_ACCEL_X_H, buf, 14) != 0)
        return MPU6050_ERR_BUS;

    raw->ax   = be16(&buf[0]);
    raw->ay   = be16(&buf[2]);
    raw->az   = be16(&buf[4]);
    raw->temp = be16(&buf[6]);
    raw->gx   = be16(&buf[8]);
    raw->gy   = be16(&buf[10]);
    raw->gz   = be16(&buf[12]);
    return MPU6050_OK;
}

void mpu6050_convert(const mpu6050_raw_t *raw, mpu6050_data_t *out)
{
    out->ax_g   = raw->ax / MPU6050_ACCEL_LSB_PER_G;
    out->ay_g   = raw->ay / MPU6050_ACCEL_LSB_PER_G;
    out->az_g   = raw->az / MPU6050_ACCEL_LSB_PER_G;
    out->gx_dps = raw->gx / MPU6050_GYRO_LSB_PER_DPS;
    out->gy_dps = raw->gy / MPU6050_GYRO_LSB_PER_DPS;
    out->gz_dps = raw->gz / MPU6050_GYRO_LSB_PER_DPS;
    /* Datasheet formula: T(degC) = raw / 340 + 36.53 */
    out->temp_c = raw->temp / 340.0f + 36.53f;
}
