# Host (PC) build for unit tests. Firmware is built in STM32CubeIDE.
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror -O0 -g
INC     = -Iport -Ilib/mpu6050 -Ilib/tilt -Itest
SRC     = lib/mpu6050/mpu6050.c lib/tilt/tilt.c test/mock_i2c.c test/test_main.c

.PHONY: test clean

test: build/test_runner
	./build/test_runner

build/test_runner: $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(INC) $(SRC) -lm -o $@

clean:
	rm -rf build
