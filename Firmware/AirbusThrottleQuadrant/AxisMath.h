#pragma once
#include <stdint.h>

// Pure, Arduino-independent calibration helper. raw is a 12-bit circular angle.
// Values outside the calibrated circular arc clamp to the nearest endpoint.
inline bool axisMathCalibratedValue(uint16_t raw, uint16_t rawMin, uint16_t rawMax,
                                    uint16_t hidMin, uint16_t hidMax, bool invert,
                                    uint16_t &output) {
  if (raw > 4095 || rawMin > 4095 || rawMax > 4095 || rawMin == rawMax || hidMax < hidMin) return false;
  const uint16_t span = (uint16_t)((rawMax - rawMin) & 0x0FFF);
  if (span == 0) return false;
  const uint16_t offset = (uint16_t)((raw - rawMin) & 0x0FFF);
  uint16_t boundedOffset = offset;
  if (offset > span) {
    const uint16_t reverseFromMin = (uint16_t)((4096U - offset) & 0x0FFF);
    const uint16_t fromMin = offset < reverseFromMin ? offset : reverseFromMin;
    const uint16_t forwardFromMax = (uint16_t)((raw - rawMax) & 0x0FFF);
    const uint16_t reverseFromMax = (uint16_t)((4096U - forwardFromMax) & 0x0FFF);
    const uint16_t fromMax = forwardFromMax < reverseFromMax ? forwardFromMax : reverseFromMax;
    boundedOffset = (fromMin <= fromMax) ? 0 : span;
  }
  const uint32_t hidSpan = (uint32_t)hidMax - hidMin;
  uint32_t value = ((uint32_t)boundedOffset * hidSpan + span / 2) / span;
  if (invert) value = hidSpan - value;
  output = (uint16_t)(hidMin + value);
  return true;
}
