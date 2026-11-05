/*
 * app.c - example application for STM32.
 *
 * Call app_init() once and app_loop() inside while(1) of the CubeMX main.c:
 *
 *     /- USER CODE BEGIN 2 -/      app_init();
 *     /- USER CODE BEGIN WHILE -/  while (1) { app_loop(); }
 *
 * Every 10 ms (100 Hz) it reads the sensor, updates roll/pitch and prints
 * a line over UART2 (the ST-Link virtual COM port on Nucleo boards):
 *     roll=  12.34 pitch=  -3.21 temp=27.5
 */
#include <stdio.h>
#include "main.h"
#include "app.h"
#include "mpu6050.h"
#include "tilt.h"

extern UART_HandleTypeDef huart2;

#define SAMPLE_PERIOD_MS 10
#define FILTER_ALPHA     0.98f

static tilt_t   s_tilt;
static uint32_t s_last_tick;
static int      s_sensor_ok;

static void uart_print(const char *s, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)s, (uint16_t)len, 100);
}

/* Format a float with 2 decimals using integers only, because newlib-nano
 * (default in STM32CubeIDE) does not support %f in printf.
 * Example: -0.5 -> "-0.50", 12.345 -> " 12.34" */
static void fmt_x100(char *buf, int size, float v)
{
    long x    = (long)(v * 100.0f);
    char sign = (x < 0) ? '-' : ' ';
    if (x < 0)
        x = -x;
    snprintf(buf, (size_t)size, "%c%ld.%02ld", sign, x / 100, x % 100);
}

void app_init(void)
{
    char msg[48];
    int  err = mpu6050_init();
    int  n;

    s_sensor_ok = (err == MPU6050_OK);
    n = snprintf(msg, sizeof msg, "MPU6050 init: %s (%d)\r\n",
                 s_sensor_ok ? "OK" : "FAIL", err);
    uart_print(msg, n);

    tilt_init(&s_tilt, FILTER_ALPHA);
    s_last_tick = HAL_GetTick();
}

void app_loop(void)
{
    mpu6050_raw_t  raw;
    mpu6050_data_t data;
    char           line[64];
    uint32_t       now = HAL_GetTick();
    int            n;

    if (!s_sensor_ok || (now - s_last_tick) < SAMPLE_PERIOD_MS)
        return;

    /* Real elapsed time, so the filter stays correct if a loop is late */
    float dt = (now - s_last_tick) / 1000.0f;
    s_last_tick = now;

    if (mpu6050_read_raw(&raw) != MPU6050_OK) {
        uart_print("I2C read error\r\n", 16);
        return;
    }
    mpu6050_convert(&raw, &data);
    tilt_update(&s_tilt, &data, dt);

    {
        char r[16], p[16];
        fmt_x100(r, sizeof r, s_tilt.roll_deg);
        fmt_x100(p, sizeof p, s_tilt.pitch_deg);
        n = snprintf(line, sizeof line, "roll=%s pitch=%s\r\n", r, p);
    }
    uart_print(line, n);
}
