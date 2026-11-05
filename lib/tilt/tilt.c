#include "tilt.h"
#include <math.h>

#define RAD_TO_DEG 57.2957795f

void tilt_init(tilt_t *t, float alpha)
{
    t->roll_deg    = 0.0f;
    t->pitch_deg   = 0.0f;
    t->alpha       = alpha;
    t->initialized = 0;
}

/* Roll from gravity: angle of the Y-Z projection */
float tilt_accel_roll(const mpu6050_data_t *d)
{
    return atan2f(d->ay_g, d->az_g) * RAD_TO_DEG;
}

/* Pitch from gravity: uses all three axes so it stays valid when rolled */
float tilt_accel_pitch(const mpu6050_data_t *d)
{
    return atan2f(-d->ax_g, sqrtf(d->ay_g * d->ay_g + d->az_g * d->az_g))
           * RAD_TO_DEG;
}

void tilt_update(tilt_t *t, const mpu6050_data_t *d, float dt_s)
{
    float acc_roll  = tilt_accel_roll(d);
    float acc_pitch = tilt_accel_pitch(d);

    /* First sample: no history yet, start from the accelerometer angle */
    if (!t->initialized) {
        t->roll_deg    = acc_roll;
        t->pitch_deg   = acc_pitch;
        t->initialized = 1;
        return;
    }

    t->roll_deg  = t->alpha * (t->roll_deg  + d->gx_dps * dt_s)
                 + (1.0f - t->alpha) * acc_roll;
    t->pitch_deg = t->alpha * (t->pitch_deg + d->gy_dps * dt_s)
                 + (1.0f - t->alpha) * acc_pitch;
}
