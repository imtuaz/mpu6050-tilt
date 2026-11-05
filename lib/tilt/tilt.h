/*
 * tilt.h - roll/pitch estimation with a complementary filter
 *
 * Accelerometer: correct on average, but noisy (vibration).
 * Gyroscope    : smooth, but integrating it drifts over time.
 * Complementary filter mixes both:
 *     angle = alpha * (angle + gyro_rate * dt) + (1 - alpha) * accel_angle
 * alpha close to 1 (e.g. 0.98) = trust gyro short-term, accel long-term.
 */
#ifndef TILT_H
#define TILT_H

#include "mpu6050.h"

typedef struct {
    float roll_deg;    /* rotation around X axis */
    float pitch_deg;   /* rotation around Y axis */
    float alpha;       /* filter coefficient, 0..1 */
    int   initialized; /* 0 until the first update */
} tilt_t;

void  tilt_init(tilt_t *t, float alpha);
float tilt_accel_roll(const mpu6050_data_t *d);
float tilt_accel_pitch(const mpu6050_data_t *d);
void  tilt_update(tilt_t *t, const mpu6050_data_t *d, float dt_s);

#endif /* TILT_H */
