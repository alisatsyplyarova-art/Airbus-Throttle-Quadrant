#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "ConfigCore.h"

inline bool configStorageLoad(ConfigRecord &config) {
  ConfigRecord stored;
  EEPROM.get(0, stored);
  if (!configRecordValid(stored)) return false;
  config = stored;
  return true;
}

inline bool configStorageSave(ConfigRecord config) {
  if (!configValid(config)) return false;
  config.magic = ATQ_CONFIG_MAGIC;
  config.schema = ATQ_CONFIG_SCHEMA;
  configSetCrc(config);
  EEPROM.put(0, config);
  ConfigRecord verify;
  EEPROM.get(0, verify);
  if (!configRecordValid(verify)) return false;
  config = verify;
  return true;
}
