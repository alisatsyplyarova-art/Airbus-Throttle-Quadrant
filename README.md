# Airbus Throttle Quadrant

DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.

> **Status:** development baseline, firmware **1.0.01**. The sketch has not yet been validated on target hardware. The architecture and roadmap are documented; planned features are not necessarily implemented.

## Fixed project decisions

- Controller: Arduino Pro Micro (ATmega32U4).
- Two main throttle levers: L (left) and R (right), each with an AS5600 magnetic angle sensor.
- TCA9548A I²C multiplexer: L on channel 0, R on channel 1.
- USB HID: L → X axis, R → Y axis.
- USB Serial: diagnostics and a separate Windows configurator.
- Configurator: Python + PySide6.
- Mechanical goal: Airbus A320-style IDLE, CL, FLX/MCT and TOGA detents.
- REVERSE mechanism remains open: architecture will support an angle zone, lift switch/sensor, or separate button/switch.
- SPEED BRAKE, FLAPS and additional buttons/switches are deferred until the two main levers are stable.

## Current firmware baseline

Implemented in Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino:
- basic AS5600 reads through TCA9548A;
- basic USB HID X/Y axes;
- source-code calibration constants, inversion and filtering;
- Serial debug output.

Not yet implemented/verified:
- hardware-validated sensor error handling;
- user-facing calibration and EEPROM configuration;
- detent state machine;
- REVERSE input abstraction;
- Windows configurator;
- SPEED BRAKE, FLAPS and extra buttons.

## Hardware

- Arduino Pro Micro, ATmega32U4 (verify exact 5 V/16 MHz or 3.3 V variant).
- 2 × AS5600 modules and matching magnets.
- 1 × TCA9548A I²C multiplexer.
- Mechanical quadrant and USB cable.

Verify module voltage and I²C logic-level requirements before wiring. Do not assume all breakout boards tolerate 5 V.

Wiring: [Electronics/WIRING.md](Electronics/WIRING.md).

## Build

1. Install Arduino IDE and the matching Pro Micro/Leonardo board definition.
2. Install **Joystick by Matthew Heironimus**.
3. Open the sketch at Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino.
4. Verify board selection, SDA/SCL pins, sensor channels and electrical levels.
5. Upload and open Serial Monitor at 115200 baud.

The sketch expects TCA9548A address 0x70, L on channel 0 and R on channel 1. Compilation and hardware testing against the actual installation are still required.

## Project documents

- [Roadmap](Documentation/ROADMAP.md)
- [Firmware architecture](Documentation/FIRMWARE_ARCHITECTURE.md)
- [Wiring](Electronics/WIRING.md)
- [Changelog](CHANGELOG.md)

## Versioning and validation

The current firmware baseline is 1.0.01. Planned milestones are described in the roadmap; a milestone must not be called complete until its exit criteria have been tested. See CHANGELOG.md.

This is a hobby controller, not certified aviation equipment. Physical detents are created by the mechanism; AS5600 only measures angle.
