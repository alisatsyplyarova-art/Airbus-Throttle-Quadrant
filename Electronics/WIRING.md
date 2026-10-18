# Wiring — version 1.0.01

## I²C layout

Both AS5600 modules normally use the same fixed I²C address, `0x36`. Connect them to separate downstream channels of the TCA9548A multiplexer.

| Arduino Pro Micro | TCA9548A |
|---|---|
| SDA | SDA |
| SCL | SCL |
| GND | GND |
| Suitable supply voltage | VIN/VCC, according to module specifications |

| Sensor | TCA9548A channel |
|---|---|
| AS5600 for Left lever (L) | SD0 / SC0 |
| AS5600 for Right lever (R) | SD1 / SC1 |

On the TCA9548A, connect each sensor's SDA/SCL to the matching channel SDA/SCL pair. Connect grounds together.

## Important power notes

- Verify the exact Pro Micro variant (5 V or 3.3 V) and the specifications of the AS5600 and TCA9548A breakout boards.
- Some breakout boards have regulators or level shifting; others do not. Do not apply 5 V to a 3.3 V-only board or expose 3.3 V-only I²C pins to 5 V pull-ups.
- Do not connect both AS5600 modules directly to the same I²C bus without a multiplexer; their identical addresses will conflict.
- Fit the AS5600 magnet as required by the sensor module, centered and at the specified gap. Check the module documentation.

## Firmware defaults

- TCA9548A address: `0x70`
- Left sensor: channel 0
- Right sensor: channel 1
- AS5600 raw angle register: `0x0C` and `0x0D`
- I²C speed: 100 kHz
