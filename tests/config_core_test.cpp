#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../Firmware/AirbusThrottleQuadrant/ConfigCore.h"

int main() {
  ConfigRecord config;
  configSetDefaults(config);
  assert(configRecordValid(config));
  assert(config.left.rawMin == 0 && config.left.rawMax == 4095);
  assert(config.right.rawMin == 0 && config.right.rawMax == 4095);
  assert(config.filterPercent == 25);

  config.left.rawMin = 4090;
  config.left.rawMax = 20; // valid wrap-around range
  config.right.invert = 1;
  config.filterPercent = 100;
  configSetCrc(config);
  assert(configRecordValid(config));

  ConfigRecord corrupt = config;
  corrupt.left.rawMin = corrupt.left.rawMax;
  configSetCrc(corrupt);
  assert(!configRecordValid(corrupt));

  corrupt = config;
  corrupt.filterPercent = 0;
  configSetCrc(corrupt);
  assert(!configRecordValid(corrupt));

  corrupt = config;
  corrupt.right.invert = 2;
  configSetCrc(corrupt);
  assert(!configRecordValid(corrupt));

  corrupt = config;
  corrupt.right.rawMax = 4096;
  configSetCrc(corrupt);
  assert(!configRecordValid(corrupt));

  corrupt = config;
  corrupt.crc ^= 0x0100;
  assert(!configRecordValid(corrupt));

  corrupt = config;
  corrupt.schema = (uint8_t)(ATQ_CONFIG_SCHEMA + 1);
  configSetCrc(corrupt);
  assert(!configRecordValid(corrupt));

  puts("ConfigCore tests passed.");
  return 0;
}
