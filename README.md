# Airbus Throttle Quadrant

DIY USB throttle quadrant for flight simulators, based on **Arduino Pro Micro (ATmega32U4)** and two **AS5600** magnetic angle sensors.

> **Status:** initial development, version **1.0.01**. Firmware is a starting point and has not yet been validated on the user's physical hardware.

## Current scope

- Two independent throttle levers: **L** (Left) and **R** (Right).
- USB HID joystick axes: L is sent as X; R is sent as Y.
- Two AS5600 sensors share I²C address `0x36`, so a **TCA9548A I²C multiplexer is required** to connect both at once.
- Adjustable end-point calibration, direction inversion and light filtering.
- Serial diagnostics for setup and calibration.

Additional SPEED BRAKE and FLAPS controls are possible later, but are not implemented in 1.0.01.

## Hardware

- Arduino Pro Micro, ATmega32U4, 5 V / 16 MHz version recommended.
- 2 × AS5600 magnetic angle sensor modules, each with its magnet.
- 1 × TCA9548A I²C multiplexer.
- USB cable and mechanical throttle quadrant.

Check the exact AS5600 breakout-board voltage requirements before wiring. Do not assume every module accepts 5 V on its power or I²C pins.

## Wiring

See [Electronics/WIRING.md](Electronics/WIRING.md).

## Build

1. Install Arduino IDE.
2. Select **Arduino Leonardo** or the matching Pro Micro board definition for your board package. A common Pro Micro definition is provided by SparkFun's AVR board package.
3. Install **Joystick by Matthew Heironimus** through Library Manager.
4. Open `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.
5. Confirm the board's SDA/SCL pins and power levels; upload with the TCA9548A connected.
6. Open Serial Monitor at **115200 baud**.

The sketch expects the multiplexer at address `0x70`, with the L sensor on channel 0 and R sensor on channel 1.

## Calibration

Move each lever to its physical minimum and maximum while watching the raw sensor values in Serial Monitor. Edit `L_RAW_MIN`, `L_RAW_MAX`, `R_RAW_MIN`, and `R_RAW_MAX` in the sketch to match the actual travel. Set `L_INVERT` or `R_INVERT` to `true` if an axis moves in the wrong direction.

AS5600 readings are 12-bit values (0–4095). Calibration supports travel that crosses the 4095→0 boundary.

## Versioning

Versions use the format `major.minor.patch`, beginning at **1.0.01**. See [CHANGELOG.md](CHANGELOG.md).

## Safety and limitations

- This is a hobby USB HID project, not certified aviation equipment.
- The physical detents are mechanical; the AS5600 measures angle and does not create detents.
- The firmware has not yet been compiled against the exact local Arduino/Joystick library installation or tested on the final assembly. Treat it as a development baseline.
