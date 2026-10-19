/*
  Airbus Throttle Quadrant
  Firmware version: 1.0.05
  Controller: Arduino Pro Micro / ATmega32U4
  Sensors: 2 x AS5600 via TCA9548A I2C multiplexer

  L lever -> USB HID X axis
  R lever -> USB HID Y axis
  Engine Master 1 -> HID button 1
  Engine Master 2 -> HID button 2
  3-position selector contact A -> HID button 3
  3-position selector contact B -> HID button 4

  Required library:
    Joystick by Matthew Heironimus

  Development baseline: not yet validated on final hardware.
*/

#include <Wire.h>
#include <Joystick.h>
#include "AxisMath.h"
#include "ConfigCore.h"
#include "ConfigStorage.h"
#include <stdlib.h>
#include <string.h>

const char* FW_VERSION = "1.0.05";

// I2C addresses
const uint8_t TCA9548A_ADDR = 0x70;
const uint8_t AS5600_ADDR = 0x36;
const uint8_t AS5600_STATUS_REG = 0x0B;
const uint8_t AS5600_RAW_ANGLE_H = 0x0C;
const uint8_t AS5600_MD = 0x20;
const uint8_t AS5600_ML = 0x10;
const uint8_t AS5600_MH = 0x08;

// TCA9548A channels
const uint8_t CHANNEL_L = 0;
const uint8_t CHANNEL_R = 1;

// Physical switch pins on Arduino Pro Micro (ATmega32U4).
// Connect each switch contact between the pin and GND.
// INPUT_PULLUP means LOW = contact closed / switch ON.
const uint8_t PIN_ENGINE_MASTER_1 = 4;
const uint8_t PIN_ENGINE_MASTER_2 = 5;
const uint8_t PIN_SELECTOR_A = 6;
const uint8_t PIN_SELECTOR_B = 7;
const uint8_t SWITCH_COUNT = 4;
const uint16_t SWITCH_DEBOUNCE_MS = 25;

// Runtime configuration is shared by Serial and future OLED/Windows interfaces.
ConfigRecord config;
bool configLoadedFromEeprom = false;

const uint16_t HID_MIN = 0;
const uint16_t HID_MAX = 1023;

const uint32_t DEBUG_INTERVAL_MS = 250;
const uint32_t SENSOR_INTERVAL_MS = 10;
const uint8_t SENSOR_LOST_AFTER_FAILURES = 10;
const uint32_t I2C_TIMEOUT_US = 25000;

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  4, 0,                 // 4 buttons, no hat switches
  true, true,           // X, Y
  false, false, false,  // Z, Rx, Ry
  false, false, false,  // Rz, rudder, throttle
  false, false, false   // accelerator, brake, steering
);

struct SwitchInput {
  uint8_t pin;
  uint8_t hidButton;     // Joystick library button indices start at 0.
  bool rawState;
  bool stableState;
  uint32_t lastChangeMs;
};

SwitchInput switches[SWITCH_COUNT] = {
  { PIN_ENGINE_MASTER_1, 0, HIGH, HIGH, 0 },
  { PIN_ENGINE_MASTER_2, 1, HIGH, HIGH, 0 },
  { PIN_SELECTOR_A,      2, HIGH, HIGH, 0 },
  { PIN_SELECTOR_B,      3, HIGH, HIGH, 0 }
};

float filteredL = 0.0f;
float filteredR = 0.0f;
bool haveL = false;
bool haveR = false;
uint32_t lastDebugMs = 0;
uint32_t lastSensorReadMs = 0;
uint8_t failedReadsL = 0;
uint8_t failedReadsR = 0;
bool sensorLostLReported = false;
bool sensorLostRReported = false;
uint8_t statusL = 0;
uint8_t statusR = 0;

bool selectMuxChannel(uint8_t channel) {
  if (channel > 7) return false;
  Wire.clearWireTimeoutFlag();
  Wire.beginTransmission(TCA9548A_ADDR); Wire.write((uint8_t)(1U << channel));
  uint8_t result = Wire.endTransmission();
  if (Wire.getWireTimeoutFlag()) { Wire.clearWireTimeoutFlag(); return false; }
  return result == 0;
}
bool disableMux() {
  Wire.clearWireTimeoutFlag();
  Wire.beginTransmission(TCA9548A_ADDR); Wire.write((uint8_t)0x00);
  uint8_t result = Wire.endTransmission();
  bool timedOut = Wire.getWireTimeoutFlag(); Wire.clearWireTimeoutFlag();
  return result == 0 && !timedOut;
}
bool readAS5600Register(uint8_t reg, uint8_t &value) {
  Wire.clearWireTimeoutFlag();
  Wire.beginTransmission(AS5600_ADDR); Wire.write(reg);
  uint8_t result = Wire.endTransmission(false);
  if (Wire.getWireTimeoutFlag() || result != 0) { Wire.clearWireTimeoutFlag(); return false; }
  uint8_t received = Wire.requestFrom(AS5600_ADDR, (uint8_t)1);
  bool timedOut = Wire.getWireTimeoutFlag();
  if (timedOut || received != 1 || Wire.available() < 1) {
    Wire.clearWireTimeoutFlag(); while (Wire.available()) Wire.read(); return false;
  }
  value = Wire.read(); Wire.clearWireTimeoutFlag(); return true;
}
bool readAS5600(uint8_t channel, uint16_t &angle, uint8_t &status) {
  if (!selectMuxChannel(channel)) { disableMux(); return false; }
  bool ok = readAS5600Register(AS5600_STATUS_REG, status);
  uint8_t highByte = 0, lowByte = 0;
  if (ok) ok = readAS5600Register(AS5600_RAW_ANGLE_H, highByte);
  if (ok) ok = readAS5600Register((uint8_t)(AS5600_RAW_ANGLE_H + 1), lowByte);
  bool muxOffOk = disableMux();
  if (!ok || !muxOffOk) return false;
  angle = (((uint16_t)highByte << 8) | lowByte) & 0x0FFF; return true;
}
bool magnetDetected(uint8_t status) { return (status & AS5600_MD) != 0; }

bool calibratedValue(uint16_t raw, uint16_t rawMin, uint16_t rawMax, bool invert, uint16_t &output) {
  return axisMathCalibratedValue(raw, rawMin, rawMax, HID_MIN, HID_MAX, invert, output);
}

float smoothValue(float previous, uint16_t current, bool &initialized) {
  if (!initialized) {
    initialized = true;
    return (float)current;
  }
  return previous + FILTER_ALPHA * ((float)current - previous);
}

void updateSwitches(uint32_t now) {
  for (uint8_t i = 0; i < SWITCH_COUNT; i++) {
    bool currentRaw = digitalRead(switches[i].pin);

    if (currentRaw != switches[i].rawState) {
      switches[i].rawState = currentRaw;
      switches[i].lastChangeMs = now;
    }

    if ((now - switches[i].lastChangeMs) >= SWITCH_DEBOUNCE_MS &&
        switches[i].stableState != switches[i].rawState) {
      switches[i].stableState = switches[i].rawState;

      // INPUT_PULLUP: LOW means the switch contact is closed.
      Joystick.setButton(switches[i].hidButton,
                         switches[i].stableState == LOW ? 1 : 0);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  Wire.setWireTimeout(I2C_TIMEOUT_US, true);

  for (uint8_t i = 0; i < SWITCH_COUNT; i++) {
    pinMode(switches[i].pin, INPUT_PULLUP);
    switches[i].rawState = digitalRead(switches[i].pin);
    switches[i].stableState = switches[i].rawState;
    switches[i].lastChangeMs = millis();
  }

  Joystick.setXAxisRange(HID_MIN, HID_MAX);
  Joystick.setYAxisRange(HID_MIN, HID_MAX);
  Joystick.begin();
  // Wait for valid sensor samples before publishing axis positions.

  // Publish the initial physical switch states after Joystick.begin().
  for (uint8_t i = 0; i < SWITCH_COUNT; i++) {
    Joystick.setButton(switches[i].hidButton,
                       switches[i].stableState == LOW ? 1 : 0);
  }

  Serial.println();
  Serial.println(F("Airbus Throttle Quadrant"));
  Serial.print(F("Firmware: "));
  Serial.println(FW_VERSION);
  Serial.println(F("TCA9548A: address 0x70; L=CH0; R=CH1"));
  Serial.println(F("Switches: Engine1=D4/B1, Engine2=D5/B2, Selector A=D6/B3, B=D7/B4"));
  Serial.println(F("Selector center position should leave both contacts open."));
  Serial.println(F("I2C: 400kHz, timeout 25ms; TCA9548A channels disabled after reads."));
  Serial.println(F("Sensor fault policy: hold last valid HID axis; report loss after 10 consecutive failures."));
}

void updateSensorHealth(bool readOk, uint8_t status, uint8_t &failures,
                        bool &lostReported, const __FlashStringHelper *label) {
  bool healthy = readOk && magnetDetected(status);
  if (healthy) {
    if (lostReported) { Serial.print(label); Serial.println(F(" SENSOR_RECOVERED")); }
    failures = 0; lostReported = false;
  } else {
    if (failures < 255) failures++;
    if (failures >= SENSOR_LOST_AFTER_FAILURES && !lostReported) {
      Serial.print(label);
      Serial.println(readOk ? F(" SENSOR_LOST: MAGNET_NOT_DETECTED") : F(" SENSOR_LOST: I2C_READ_FAILURE"));
      lostReported = true;
    }
  }
}
void loop() {
  uint32_t now = millis(); updateSwitches(now);
  if (now - lastSensorReadMs < SENSOR_INTERVAL_MS) return;
  lastSensorReadMs = now;
  uint16_t rawL=0, rawR=0, targetL=0, targetR=0;
  bool readOkL=readAS5600(CHANNEL_L,rawL,statusL), readOkR=readAS5600(CHANNEL_R,rawR,statusR);
  bool healthyL=readOkL && magnetDetected(statusL), healthyR=readOkR && magnetDetected(statusR);
  updateSensorHealth(readOkL,statusL,failedReadsL,sensorLostLReported,F("L"));
  updateSensorHealth(readOkR,statusR,failedReadsR,sensorLostRReported,F("R"));
  bool calOkL=healthyL && calibratedValue(rawL,L_RAW_MIN,L_RAW_MAX,L_INVERT,targetL);
  bool calOkR=healthyR && calibratedValue(rawR,R_RAW_MIN,R_RAW_MAX,R_INVERT,targetR);
  if (calOkL) { filteredL=smoothValue(filteredL,targetL,haveL); Joystick.setXAxis((int)filteredL); }
  if (calOkR) { filteredR=smoothValue(filteredR,targetR,haveR); Joystick.setYAxis((int)filteredR); }

  if (now-lastDebugMs >= DEBUG_INTERVAL_MS) {
    lastDebugMs=now;
    Serial.print(F("L: "));
    if (!readOkL) { Serial.print(F("ERR_READ; HID HOLD=")); Serial.print(haveL?(int)filteredL:-1); }
    else if (!magnetDetected(statusL)) { Serial.print(F("ERR_MAGNET_MD=0; HID HOLD=")); Serial.print(haveL?(int)filteredL:-1); }
    else if (!calOkL) { Serial.print(F("ERR_CAL; RAW=")); Serial.print(rawL); Serial.print(F("; HID HOLD=")); Serial.print((int)filteredL); }
    else { Serial.print(rawL); Serial.print(F(" -> ")); Serial.print((int)filteredL); }
    if (readOkL && (statusL & AS5600_ML)) Serial.print(F(" WARN_MAGNET_WEAK"));
    if (readOkL && (statusL & AS5600_MH)) Serial.print(F(" WARN_MAGNET_STRONG"));
    Serial.print(F(" | R: "));
    if (!readOkR) { Serial.print(F("ERR_READ; HID HOLD=")); Serial.print(haveR?(int)filteredR:-1); }
    else if (!magnetDetected(statusR)) { Serial.print(F("ERR_MAGNET_MD=0; HID HOLD=")); Serial.print(haveR?(int)filteredR:-1); }
    else if (!calOkR) { Serial.print(F("ERR_CAL; RAW=")); Serial.print(rawR); Serial.print(F("; HID HOLD=")); Serial.print((int)filteredR); }
    else { Serial.print(rawR); Serial.print(F(" -> ")); Serial.print((int)filteredR); }
    if (readOkR && (statusR & AS5600_ML)) Serial.print(F(" WARN_MAGNET_WEAK"));
    if (readOkR && (statusR & AS5600_MH)) Serial.print(F(" WARN_MAGNET_STRONG"));
    Serial.print(F(" | SW E1=")); Serial.print(switches[0].stableState==LOW?F("ON"):F("OFF"));
    Serial.print(F(" E2=")); Serial.print(switches[1].stableState==LOW?F("ON"):F("OFF"));
    Serial.print(F(" SEL_A=")); Serial.print(switches[2].stableState==LOW?F("1"):F("0"));
    Serial.print(F(" SEL_B=")); Serial.print(switches[3].stableState==LOW?F("1"):F("0"));
    Serial.println();
  }
}
