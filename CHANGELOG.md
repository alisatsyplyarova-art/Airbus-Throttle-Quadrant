# Changelog

## Unreleased — architecture and roadmap fixed (2026-10-19)

- Recorded MSFS as the target simulator and Arduino Pro Micro ATmega32U4 as the controller.
- Confirmed L/R throttle levers, AS5600 sensors via TCA9548A, and USB HID axis mapping L → X, R → Y.
- Selected Python + PySide6 for the planned Windows configurator using USB Serial.
- Kept REVERSE mechanism undecided by design; future input abstraction will support angle zone, lift switch/sensor, separate button/switch, or disabled mode.
- Deferred SPEED BRAKE, FLAPS and additional buttons/switches until the two main levers are stable.
- Added the development roadmap and firmware architecture document.

**No new firmware feature is claimed as implemented or hardware-validated by this planning update.**

## 1.0.01 — Initial development baseline

- Initial repository structure and documentation.
- Two throttle axes named L and R in project documentation.
- AS5600 sensors read through TCA9548A I²C multiplexer.
- USB HID joystick X/Y axes, configurable calibration and inversion.
- Serial diagnostics and basic input filtering.

**Validation:** not yet hardware-validated. Compile and test with the exact Arduino Pro Micro board package and Joystick library before relying on this firmware.
