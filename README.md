# Sumo Robot

An ESP32-based sumo-wrestling robot that detects an opponent with a
time-of-flight depth sensor and autonomously attacks or searches for a target.

## Overview

The robot scans the area in front of it with a SparkFun VL53L5CX 8x8 zone
time-of-flight sensor. When an opponent is detected within range in the
central zones of the sensor's field of view, the robot drives straight ahead
to attack; otherwise it rotates in place to search. A physical start switch
gates the robot with a 5-second start delay (standard for sumo competitions)
and an immediate stop.

## Hardware

- ESP32 development board
- SparkFun VL53L5CX time-of-flight 8x8 zone sensor (I2C)
  - SDA on GPIO 21, SCL on GPIO 22 (`platform.h`)
- Dual DC motor driver, controlled via 4 PWM-capable GPIOs:
  - Motor 1: GPIO 25 (IN1), GPIO 26 (IN2)
  - Motor 2: GPIO 27 (IN1), GPIO 14 (IN2)
- Start/enable switch on GPIO 18 (input)
- Status/indicator pin on GPIO 5 (output)

## Behavior

1. On power-up, the robot waits for the start switch (GPIO 18) to be high.
2. Once enabled, it waits 5 seconds before becoming active (competition start
   delay).
3. While active, it continuously polls the VL53L5CX for ranging data:
   - If an opponent is detected within range in the central zones, it drives
     forward to attack.
   - Otherwise, it rotates in place to search for a target.
4. Pulling the start switch low immediately stops both motors and resets the
   robot to its idle state.

## Project structure

```
sumo-robot/
├── robot_drive_logic.cpp       # Arduino sketch: setup()/loop(), motor control, detection logic
└── src/
    ├── platform.h / .cpp       # I2C platform glue required by the VL53L5CX driver
    ├── SparkFun_VL53L5CX_Library.h/.cpp     # SparkFun VL53L5CX high-level driver
    ├── SparkFun_VL53L5CX_IO.h/.cpp          # SparkFun VL53L5CX I2C I/O layer
    ├── vl53l5cx_api.h/.cpp                  # ST VL53L5CX Ultra Lite Driver API
    ├── vl53l5cx_buffers.h                   # ST driver firmware buffers
    ├── vl53l5cx_plugin_*.h/.cpp             # ST driver optional plugins (xtalk, motion, thresholds)
    └── SparkFun_VL53L5CX_Library_Constants.h
```

## Dependencies

- [Arduino core for ESP32](https://github.com/espressif/arduino-esp32)
- [SparkFun VL53L5CX Arduino Library](https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library) (bundled in `src/`, built on ST's VL53L5CX Ultra Lite Driver)

## Building and flashing

This is an Arduino-framework sketch for ESP32, set up as a CLion project.
Build and upload it with your preferred Arduino/ESP32 toolchain (Arduino IDE,
PlatformIO, or `arduino-cli`), using `robot_drive_logic.cpp` as the sketch
entry point and `src/` for the supporting sensor driver.

## Tuning

Key constants in `robot_drive_logic.cpp`:

- `maxSpeed` / `rotateSpeed` — motor PWM duty cycle for attack vs. search modes
- The detection distance threshold (in millimeters) used when deciding
  whether an opponent is within attack range
- The sensor zone rows/columns scanned for detection, matching the
  VL53L5CX's 8x8 zone grid

## License

The VL53L5CX driver and SparkFun library files retain their original
licenses (ST Microelectronics proprietary / BSD-3-Clause dual license, and
SparkFun's library license respectively) — see the headers in `src/`.
