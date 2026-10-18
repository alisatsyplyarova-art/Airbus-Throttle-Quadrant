# Roadmap — Airbus Throttle Quadrant

**Project target:** DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.
**Architecture baseline:** 2026-10-19
**Current firmware baseline:** 1.0.01 (development only; not yet validated on target hardware).

## Decisions fixed for this roadmap

- Main controller: Arduino Pro Micro, ATmega32U4.
- Main levers: L (left) and R (right), each measured by an AS5600 magnetic angle sensor.
- Both AS5600 sensors normally use I²C address 0x36, so use a TCA9548A multiplexer: L on channel 0, R on channel 1.
- USB HID is the simulator-facing interface. Current mapping: L → X axis, R → Y axis.
- USB Serial is reserved for diagnostics and a separate Windows configurator.
- Configurator: Python + PySide6.
- Mechanical goal: Airbus A320-style detents IDLE, CL, FLX/MCT and TOGA. Exact geometry and calibration values must be measured from the physical mechanism.
- REVERSE implementation is intentionally undecided. Architecture must support a dedicated angle zone, a separate lift-detection switch/sensor, or a separate button/switch.
- SPEED BRAKE, FLAPS and additional buttons/switches are deferred until the two main levers are stable.
- Versions below are planned milestones, not a claim that features are already implemented.

## Milestones

### 1.0.02 — Reliable sensor and HID baseline
- Compile with the exact target Pro Micro board definition and selected Joystick library.
- Verify both AS5600 channels through TCA9548A.
- Report sensor errors explicitly and define safe behaviour after loss of a sensor; do not silently treat stale data as healthy.
- Test circular angle handling across 4095 → 0.
- Verify HID axis ranges, direction, update stability and USB reconnection.
- Record hardware/software test results.

**Exit criteria:** repeatable readings and stable L/R axes on actual hardware; errors are visible in diagnostics.

### 1.0.03 — Calibration and configuration storage
- Define a versioned configuration structure.
- Calibrate each axis minimum, maximum and direction without editing source code.
- Validate ranges and handle wrap-around correctly.
- Save settings to EEPROM with a signature/version and integrity check.
- Add Serial commands to read status/configuration, change values, save and restore defaults.

**Exit criteria:** settings survive power cycling; invalid settings are rejected safely.

### 1.1.00 — Throttle detents and reverse input abstraction
- Implement configurable detent thresholds and hysteresis.
- Support IDLE, CL, FLX/MCT and TOGA.
- Add a ReverseInput abstraction supporting angle-zone, lift switch/sensor, button/switch, or disabled mode.
- Test mappings with the intended MSFS aircraft and document aircraft-specific assignment requirements.

**Exit criteria:** repeatable detent recognition and documented behaviour in the chosen simulator aircraft.

### 1.2.00 — Windows configurator
- Build a separate Python + PySide6 application.
- Connect over USB Serial without blocking USB HID operation.
- Display firmware version, sensor health and live L/R values.
- Provide axis calibration, direction, filter and detent settings.
- Import/export configuration profiles and show clear command errors.

**Exit criteria:** app can read, validate, change, save and restore settings on actual controller.

### 1.3.00 — Additional controls
- Add SPEED BRAKE and FLAPS as separate physical controls after the main levers are validated.
- Select sensors/switches after the mechanical design is known.
- Add button/toggle inputs and debounce handling.
- Extend HID reports and configurator without destabilising L/R axes.

**Exit criteria:** all new controls are testable in Windows and assignable in MSFS.

### 2.0.00 — Mechanical integration and release preparation
- Publish mechanical drawings/STL files when validated.
- Publish wiring, assembly and calibration instructions.
- Document firmware, configurator and configuration-schema versions.
- Run a full regression checklist and provide known limitations.

## Working rules

- Do not mark a milestone complete until its exit criteria are tested.
- Keep each firmware change small and update CHANGELOG.md.
- Do not claim hardware validation without actual test results.
- Keep SPEED BRAKE and FLAPS out of the first stable firmware milestone.
