/*
 * mpu6050.h - minimal MPU6050 driver (accelerometer + gyroscope + temperature)
 *
 * Full-scale settings used:
 *   Accelerometer: +-2 g      -> 16384 LSB per g
 *   Gyroscope    : +-250 dps  -> 131   LSB per degree/second
 */
#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

#define MPU6050_ADDR          0x68  /* AD0 pin = GND (0x69 if AD0 = VCC) */

/* Register map (only what we use) */
#define MPU6050_REG_GYRO_CFG  0x1B
#define MPU6050_REG_ACCEL_CFG 0x1C
#define MPU6050_REG_ACCEL_X_H 0x3B  /* first of 14 data bytes */
#define MPU6050_REG_PWR_MGMT1 0x6B
#define MPU6050_REG_WHO_AM_I  0x75
#define MPU6050_WHO_AM_I_VAL  0x68

#define MPU6050_ACCEL_LSB_PER_G   16384.0f
#define MPU6050_GYRO_LSB_PER_DPS  131.0f

/* Error codes */
#define MPU6050_OK         0
#define MPU6050_ERR_BUS   -1   /* I2C transaction failed      */
#define MPU6050_ERR_ID    -2   /* WHO_AM_I mismatch (wrong chip / wiring) */

/* Raw 16-bit values exactly as read from the sensor */
typedef struct {
    int16_t ax, ay, az;
    int16_t temp;
    int16_t gx, gy, gz;
} mpu6050_raw_t;

/* Values converted to physical units */
typedef struct {
    float ax_g, ay_g, az_g;        /* acceleration in g      */
    float gx_dps, gy_dps, gz_dps;  /* angular rate in deg/s  */
    float temp_c;                  /* temperature in deg C   */
} mpu6050_data_t;

/* Gyro zero-rate offset (deg/s), measured while the board is still */
typedef struct {
    float gx_dps, gy_dps, gz_dps;
} mpu6050_gyro_bias_t;

#define MPU6050_ERR_ARG   -3   /* invalid argument */

int  mpu6050_init(void);
int  mpu6050_read_raw(mpu6050_raw_t *raw);
void mpu6050_convert(const mpu6050_raw_t *raw, mpu6050_data_t *out);

/* Average `samples` gyro readings to estimate the bias.
 * The board MUST be stationary during calibration. */
int  mpu6050_calibrate_gyro(int samples, mpu6050_gyro_bias_t *bias);
void mpu6050_apply_gyro_bias(mpu6050_data_t *d, const mpu6050_gyro_bias_t *bias);

#endif /* MPU6050_H */
