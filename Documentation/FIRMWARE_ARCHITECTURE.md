# Firmware Architecture

**Target MCU:** Arduino Pro Micro, ATmega32U4
**Simulator:** Microsoft Flight Simulator
**Control link:** USB HID
**Configuration/diagnostics link:** USB Serial
**Status:** planned design; implementation is incremental.

## 1. Data flow

1. Inputs: AS5600 L/R through TCA9548A; future discrete switches and sensors.
2. Sensor layer selects a multiplexer channel, reads raw angle and reports validity/communication errors.
3. Axis processing unwraps/calibrates physical travel, applies inversion and bounded filtering, and produces normalized axis values.
4. Control interpretation detects throttle detents with hysteresis and resolves the configured reverse-input source.
5. Output layer sends axes/buttons via USB HID and status/configuration responses via Serial.
6. Configuration storage validates and persists settings in EEPROM.

Sensor validity must be separate from the last measured value. If a sensor read fails, firmware must report the fault and apply a documented fallback policy rather than silently presenting stale data as healthy. The precise HID fallback must be chosen and tested before milestone 1.0.02 is complete.

## 2. Suggested module boundaries

| Module | Responsibility |
|---|---|
| SensorManager | TCA9548A channel selection, AS5600 reads, validity and error counters |
| AxisProcessor | Calibration, circular travel handling, inversion, filtering, normalized values |
| DetentManager | IDLE, CL, FLX/MCT and TOGA states; configurable thresholds and hysteresis |
| ReverseInput | Selectable source: ANGLE_ZONE, LIFT_SWITCH, BUTTON, DISABLED |
| ButtonManager | Future switches/buttons, debounce and logical states |
| HidManager | USB HID axis and button reports |
| ConfigStorage | Versioned settings, validation and EEPROM persistence |
| SerialProtocol | Commands, responses, diagnostics and configurator communication |

These are logical boundaries first; files may be split gradually after the existing sketch is audited and compiling.

## 3. Main axis model

Each throttle lever has independent configuration:
- raw minimum and maximum;
- direction inversion;
- filter parameters;
- optional detent thresholds.

AS5600 output is 12-bit (0–4095). Since the reading is circular, calibration must correctly handle a physical range that crosses 4095 → 0. Equal endpoints must be treated as invalid configuration rather than silently assumed to mean a full revolution.

Current USB HID mapping:
- lever L → X axis;
- lever R → Y axis.

The final HID range and axis behaviour must be confirmed in Windows and MSFS.

## 4. Detent model

Detents are logical states derived from calibrated position; they do not physically create the mechanical stops. Physical detents belong to the mechanical design.

Each detent should have a configurable centre/threshold and hysteresis (separate entry/exit boundary or equivalent stateful rule) to prevent flicker around a boundary. Exact angles are not hard-coded until the finished mechanism can be measured.

Initial states to support: IDLE, CL, FLX_MCT, TOGA. REVERSE remains a separately configurable input source and must not be coupled irreversibly to the main angle calculation.

## 5. Configuration protocol

Use newline-delimited text commands over USB Serial for the first implementation. The protocol is provisional and must be versioned before the PC application depends on it.

Planned command groups:
- status/version: GET_STATUS, GET_CONFIG;
- axis configuration: read/set minimum, maximum, inversion and filter;
- detent configuration: read/set thresholds;
- persistence: SAVE_CONFIG, LOAD_DEFAULTS.

Every command needs a clear success/error response. Validate input bounds, reject unknown commands safely and keep the sensor/HID loop non-blocking. Only explicit save operations should write EEPROM. The exact grammar, response schema and maximum line length must be specified during implementation.

## 6. EEPROM configuration

Use a structure with a signature, schema version and integrity check. On startup:
1. read stored data;
2. verify signature, version, ranges and integrity;
3. use valid configuration or load safe defaults;
4. report invalid/missing configuration over Serial.

Avoid unnecessary writes. Plan for future schema migration when fields are added.

## 7. Windows configurator

Planned stack: Python + PySide6, with a serial-port library such as pyserial.

Initial screens:
- connection and firmware/status;
- live L/R readings and sensor health;
- axis calibration and inversion;
- detent configuration;
- configuration profiles (import/export);
- diagnostics and command errors.

The configurator communicates only through the documented Serial protocol. Closing the app or losing the Serial connection must not stop USB HID control.

## 8. Future inputs

SPEED BRAKE, FLAPS and extra buttons are deliberately deferred until L/R stability and calibration are complete. Their hardware interfaces should be added only after mechanical travel, sensor type and electrical levels are known. The architecture should allow extra axes and button inputs without changing the core L/R sensor code.

## 9. Validation requirements

- Compile with the documented board package and exact Joystick library.
- Test both sensor channels and deliberate sensor disconnection.
- Test angle wrap-around and endpoints.
- Confirm axis direction/range in Windows game-controller properties.
- Confirm assignments and detent behaviour in the target MSFS aircraft.
- Power-cycle and verify EEPROM configuration.
- Test malformed Serial commands and configurator disconnects.
- Record actual results; never describe untested firmware as hardware-validated.
