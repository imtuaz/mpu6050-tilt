/*
 * test_main.c - unit tests that run on a PC (no board needed).
 * Build and run with:  make test
 */
#include <stdio.h>
#include <math.h>
#include "mpu6050.h"
#include "tilt.h"
#include "mock_i2c.h"

static int g_fail = 0;
static int g_pass = 0;

#define CHECK(cond) do {                                            \
        if (cond) { g_pass++; }                                     \
        else { g_fail++; printf("  FAIL %s:%d: %s\n",               \
                                __FILE__, __LINE__, #cond); }       \
    } while (0)

#define CHECK_NEAR(a, b, tol) CHECK(fabsf((a) - (b)) <= (tol))

/* ---- driver tests ---------------------------------------------------- */

static void test_init_ok(void)
{
    mock_reset();
    CHECK(mpu6050_init() == MPU6050_OK);
    CHECK(mock_regs[MPU6050_REG_PWR_MGMT1] == 0x00);   /* woke up */
    CHECK(mock_regs[MPU6050_REG_ACCEL_CFG] == 0x00);   /* +-2 g    */
    CHECK(mock_regs[MPU6050_REG_GYRO_CFG]  == 0x00);   /* +-250 dps */
}

static void test_init_wrong_id(void)
{
    mock_reset();
    mock_regs[MPU6050_REG_WHO_AM_I] = 0x12;            /* wrong chip */
    CHECK(mpu6050_init() == MPU6050_ERR_ID);
    CHECK(mock_regs[MPU6050_REG_PWR_MGMT1] == 0x40);   /* untouched */
}

static void test_init_bus_error(void)
{
    mock_reset();
    mock_fail_bus = 1;
    CHECK(mpu6050_init() == MPU6050_ERR_BUS);
}

static void test_read_and_convert(void)
{
    mpu6050_raw_t  raw;
    mpu6050_data_t d;

    mock_reset();
    mock_set16(0x3B,  16384);   /* ax = +1 g                */
    mock_set16(0x3D, -8192);    /* ay = -0.5 g (negative!)  */
    mock_set16(0x3F,  0);       /* az = 0                   */
    mock_set16(0x41,  0);       /* temp raw 0 -> 36.53 C    */
    mock_set16(0x43,  131);     /* gx = +1 dps              */
    mock_set16(0x45, -262);     /* gy = -2 dps              */
    mock_set16(0x47,  32767);   /* gz = max positive        */

    CHECK(mpu6050_read_raw(&raw) == MPU6050_OK);
    CHECK(raw.ax == 16384);
    CHECK(raw.ay == -8192);
    CHECK(raw.gz == 32767);

    mpu6050_convert(&raw, &d);
    CHECK_NEAR(d.ax_g,    1.0f,   1e-4f);
    CHECK_NEAR(d.ay_g,   -0.5f,   1e-4f);
    CHECK_NEAR(d.gx_dps,  1.0f,   1e-4f);
    CHECK_NEAR(d.gy_dps, -2.0f,   1e-4f);
    CHECK_NEAR(d.temp_c, 36.53f,  1e-3f);
}

/* ---- tilt tests ------------------------------------------------------ */

static void test_accel_angles(void)
{
    mpu6050_data_t d = {0};

    d.az_g = 1.0f;                           /* lying flat */
    CHECK_NEAR(tilt_accel_roll(&d),  0.0f, 0.01f);
    CHECK_NEAR(tilt_accel_pitch(&d), 0.0f, 0.01f);

    d.ay_g = 1.0f; d.az_g = 1.0f;            /* rolled 45 deg */
    CHECK_NEAR(tilt_accel_roll(&d), 45.0f, 0.01f);

    d.ax_g = -1.0f; d.ay_g = 0.0f; d.az_g = 0.0f;   /* pitched up 90 deg */
    CHECK_NEAR(tilt_accel_pitch(&d), 90.0f, 0.01f);
}

static void test_filter_first_sample(void)
{
    tilt_t t;
    mpu6050_data_t d = {0};

    tilt_init(&t, 0.98f);
    d.ay_g = 1.0f; d.az_g = 1.0f;            /* 45 deg roll */
    tilt_update(&t, &d, 0.01f);
    CHECK_NEAR(t.roll_deg, 45.0f, 0.01f);    /* starts from accel angle */
}

static void test_filter_converges(void)
{
    tilt_t t;
    mpu6050_data_t flat = {0}, tilted = {0};

    tilt_init(&t, 0.98f);
    flat.az_g = 1.0f;
    tilt_update(&t, &flat, 0.01f);           /* start at 0 deg */

    /* Board now held still at 30 deg roll, gyro reads 0 */
    tilted.ay_g = sinf(30.0f / 57.2957795f);
    tilted.az_g = cosf(30.0f / 57.2957795f);

    tilt_update(&t, &tilted, 0.01f);
    CHECK(t.roll_deg > 0.0f && t.roll_deg < 2.0f);  /* moves slowly, no jump */

    for (int i = 0; i < 500; i++)            /* 5 seconds at 100 Hz */
        tilt_update(&t, &tilted, 0.01f);
    CHECK_NEAR(t.roll_deg, 30.0f, 0.1f);     /* settles on true angle */
}

static void test_filter_rejects_accel_spike(void)
{
    tilt_t t;
    mpu6050_data_t flat = {0}, spike = {0};

    tilt_init(&t, 0.98f);
    flat.az_g = 1.0f;
    tilt_update(&t, &flat, 0.01f);

    spike.ay_g = 1.0f; spike.az_g = 0.0f;   /* one noisy sample says 90 deg */
    tilt_update(&t, &spike, 0.01f);
    CHECK(t.roll_deg < 2.0f);               /* filter barely reacts: 0.02*90 */
}

int main(void)
{
    printf("Running tests...\n");
    test_init_ok();
    test_init_wrong_id();
    test_init_bus_error();
    test_read_and_convert();
    test_accel_angles();
    test_filter_first_sample();
    test_filter_converges();
    test_filter_rejects_accel_spike();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
