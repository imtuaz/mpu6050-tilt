# MPU6050 Tilt Sensor (STM32, I2C, Complementary Filter)

Reads an MPU6050 IMU over I2C, estimates **roll** and **pitch** with a
complementary filter at 100 Hz, and prints the angles over UART.

The driver and filter are **hardware-independent** and covered by unit tests
that run on a PC, so the logic is verified before flashing any board.

## Architecture
```
         app.c  (100 Hz loop, UART output)
           |
   +-------+--------+
   |                |
mpu6050.c        tilt.c         <- portable C, no HAL calls
   |
i2c_port.h      (3 functions: write_reg, read_regs, delay_ms)
   |
   +-- port/stm32/i2c_port_stm32.c   real board (STM32 HAL)
   +-- test/mock_i2c.c               PC tests (fake registers)
```

## Folder structure
```
lib/mpu6050/   sensor driver: init, burst read, unit conversion
lib/tilt/      roll/pitch from accel + complementary filter
port/          hardware abstraction layer
port/stm32/    STM32 HAL implementation + example app
test/          mock I2C bus + unit tests (run on PC)
```

## Run unit tests (no board needed)
```bash
make test
```
Expected: `37 passed, 0 failed`

What is tested:
- Init: wake-up and range registers written; wrong `WHO_AM_I` and bus errors detected
- Big-endian decoding of negative and maximum values
- Unit conversion (g, deg/s, deg C)
- Accelerometer angles at 0, 45 and 90 degrees
- Filter: starts from accel angle, converges to true angle, rejects a single noise spike
- Gyro calibration: bias averaging, no overflow at 1000 x max value, error handling,
  and removal of the steady angle error caused by a 1 deg/s offset

## Use on STM32 (e.g. Nucleo-F401RE / Blue Pill)
1. In CubeMX enable **I2C1** (100 kHz) and **USART2** (115200 8N1).
2. Copy `lib/` and `port/` into the project and add their include paths.
3. In `main.c`:
   ```c
   #include "app.h"
   /* USER CODE BEGIN 2 */     app_init();
   /* inside while (1) */      app_loop();
   ```
4. Open a serial terminal at 115200 baud.
5. **Keep the board still for about 1 s after reset** while the gyro is calibrated.

## Wiring
| MPU6050 | STM32 (I2C1 default) |
|---------|----------------------|
| VCC     | 3.3 V                |
| GND     | GND                  |
| SCL     | PB6                  |
| SDA     | PB7                  |
| AD0     | GND (address 0x68)   |

## Filter
```
angle = alpha * (angle + gyro_rate * dt) + (1 - alpha) * accel_angle
```
`alpha = 0.98`: gyro dominates short-term (smooth), accelerometer corrects
long-term drift. With dt = 10 ms the time constant is about
`alpha * dt / (1 - alpha) = 0.49 s`.

## Limitations
- Gyro bias is measured once at startup; it does not track temperature drift
- Yaw is not estimated (needs a magnetometer)
- Blocking I2C/UART calls; DMA/interrupt version is a possible next step
