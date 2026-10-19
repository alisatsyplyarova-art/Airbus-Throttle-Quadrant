/*
  Airbus Throttle Quadrant
  Firmware version: 1.0.03
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

const char* FW_VERSION = "1.0.03";

// I2C addresses
const uint8_t TCA9548A_ADDR = 0x70;
const uint8_t AS5600_ADDR = 0x36;
const uint8_t AS5600_RAW_ANGLE_H = 0x0C;

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

// Calibrate these values to the actual mechanical travel (0..4095).
// The circular range may cross the 4095 -> 0 boundary.
// Equal endpoints are invalid and do not represent a full revolution.
const uint16_t L_RAW_MIN = 0;
const uint16_t L_RAW_MAX = 4095;
const uint16_t R_RAW_MIN = 0;
const uint16_t R_RAW_MAX = 4095;

const bool L_INVERT = false;
const bool R_INVERT = false;

const uint16_t HID_MIN = 0;
const uint16_t HID_MAX = 1023;

// EMA alpha: 1 = no filtering; smaller values = smoother/slower (0 would freeze the output).
const float FILTER_ALPHA = 0.25f;
const uint32_t DEBUG_INTERVAL_MS = 250;

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

float filteredL = 512.0f;
float filteredR = 512.0f;
bool haveL = false;
bool haveR = false;
uint32_t lastDebugMs = 0;

bool selectMuxChannel(uint8_t channel) {
  if (channel > 7) return false;
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write((uint8_t)(1U << channel));
  return Wire.endTransmission() == 0;
}

bool readAS5600(uint8_t channel, uint16_t &angle) {
  if (!selectMuxChannel(channel)) return false;

  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_RAW_ANGLE_H);
  if (Wire.endTransmission(false) != 0) return false;

  uint8_t received = Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (received != 2 || Wire.available() < 2) return false;

  uint8_t highByte = Wire.read();
  uint8_t lowByte = Wire.read();
  angle = (((uint16_t)highByte << 8) | lowByte) & 0x0FFF;
  return true;
}

// Convert a circular 12-bit angle to a linear calibrated travel value.
// Returns false for invalid calibration instead of treating equal endpoints
// as an almost-full revolution.
bool calibratedValue(
  uint16_t raw,
  uint16_t rawMin,
  uint16_t rawMax,
  bool invert,
  uint16_t &output
) {
  if (rawMin > 4095 || rawMax > 4095 || rawMin == rawMax) return false;

  raw &= 0x0FFF;
  uint16_t span = (rawMax - rawMin) & 0x0FFF;
  if (span == 0) return false;

  uint16_t offset = (raw - rawMin) & 0x0FFF;
  if (offset > span) offset = span;

  uint32_t value = ((uint32_t)offset * (HID_MAX - HID_MIN) + span / 2) / span;
  if (invert) value = HID_MAX - value;
  output = (uint16_t)(HID_MIN + value);
  return true;
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
  Wire.setClock(100000);

  for (uint8_t i = 0; i < SWITCH_COUNT; i++) {
    pinMode(switches[i].pin, INPUT_PULLUP);
    switches[i].rawState = digitalRead(switches[i].pin);
    switches[i].stableState = switches[i].rawState;
    switches[i].lastChangeMs = millis();
  }

  Joystick.setXAxisRange(HID_MIN, HID_MAX);
  Joystick.setYAxisRange(HID_MIN, HID_MAX);
  Joystick.begin();
  // Start from a known HID value so the fault policy's initial HOLD value is real.
  Joystick.setXAxis((int)filteredL);
  Joystick.setYAxis((int)filteredR);

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
  Serial.println(F("Sensor fault policy: hold last HID axis value and report fault."));
}

void loop() {
  uint16_t rawL = 0;
  uint16_t rawR = 0;
  uint16_t targetL = 0;
  uint16_t targetR = 0;

  bool readOkL = readAS5600(CHANNEL_L, rawL);
  bool readOkR = readAS5600(CHANNEL_R, rawR);
  bool calOkL = readOkL && calibratedValue(rawL, L_RAW_MIN, L_RAW_MAX, L_INVERT, targetL);
  bool calOkR = readOkR && calibratedValue(rawR, R_RAW_MIN, R_RAW_MAX, R_INVERT, targetR);

  if (calOkL) {
    filteredL = smoothValue(filteredL, targetL, haveL);
    Joystick.setXAxis((int)filteredL);
  }

  if (calOkR) {
    filteredR = smoothValue(filteredR, targetR, haveR);
    Joystick.setYAxis((int)filteredR);
  }

  uint32_t now = millis();
  updateSwitches(now);

  if (now - lastDebugMs >= DEBUG_INTERVAL_MS) {
    lastDebugMs = now;

    Serial.print(F("L: "));
    if (!readOkL) {
      Serial.print(F("ERR_READ; HID HOLD="));
      Serial.print((int)filteredL);
    } else if (!calOkL) {
      Serial.print(F("ERR_CAL; RAW="));
      Serial.print(rawL);
      Serial.print(F("; HID HOLD="));
      Serial.print((int)filteredL);
    } else {
      Serial.print(rawL);
      Serial.print(F(" -> "));
      Serial.print((int)filteredL);
    }

    Serial.print(F(" | R: "));
    if (!readOkR) {
      Serial.print(F("ERR_READ; HID HOLD="));
      Serial.print((int)filteredR);
    } else if (!calOkR) {
      Serial.print(F("ERR_CAL; RAW="));
      Serial.print(rawR);
      Serial.print(F("; HID HOLD="));
      Serial.print((int)filteredR);
    } else {
      Serial.print(rawR);
      Serial.print(F(" -> "));
      Serial.print((int)filteredR);
    }

    Serial.print(F(" | SW E1="));
    Serial.print(switches[0].stableState == LOW ? F("ON") : F("OFF"));
    Serial.print(F(" E2="));
    Serial.print(switches[1].stableState == LOW ? F("ON") : F("OFF"));
    Serial.print(F(" SEL_A="));
    Serial.print(switches[2].stableState == LOW ? F("1") : F("0"));
    Serial.print(F(" SEL_B="));
    Serial.print(switches[3].stableState == LOW ? F("1") : F("0"));
    Serial.println();
  }

  delay(5);
}
