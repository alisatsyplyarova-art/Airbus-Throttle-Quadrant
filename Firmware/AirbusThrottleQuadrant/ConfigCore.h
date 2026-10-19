#pragma once
#include <stdint.h>
#include <stddef.h>

// Stable, byte-packed EEPROM record. Keep schema changes explicit.
#define ATQ_CONFIG_MAGIC 0x4154u
#define ATQ_CONFIG_SCHEMA 1u

#pragma pack(push, 1)
struct AxisConfig {
  uint16_t rawMin;
  uint16_t rawMax;
  uint8_t invert;
};
struct ConfigRecord {
  uint16_t magic;
  uint8_t schema;
  AxisConfig left;
  AxisConfig right;
  uint8_t filterPercent; // EMA alpha, 1..100
  uint16_t crc;
};
#pragma pack(pop)

inline uint16_t configCrc16(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFFu;
  for (size_t i = 0; i < length; ++i) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u) : (uint16_t)(crc << 1);
    }
  }
  return crc;
}

inline void configSetCrc(ConfigRecord &config) {
  config.crc = configCrc16(reinterpret_cast<const uint8_t *>(&config), offsetof(ConfigRecord, crc));
}

inline bool configAxisValid(const AxisConfig &axis) {
  return axis.rawMin <= 4095u && axis.rawMax <= 4095u &&
         axis.rawMin != axis.rawMax && axis.invert <= 1u;
}

inline bool configValid(const ConfigRecord &config) {
  return config.magic == ATQ_CONFIG_MAGIC &&
         config.schema == ATQ_CONFIG_SCHEMA &&
         configAxisValid(config.left) &&
         configAxisValid(config.right) &&
         config.filterPercent >= 1u && config.filterPercent <= 100u;
}

inline bool configCrcValid(const ConfigRecord &config) {
  return config.crc == configCrc16(reinterpret_cast<const uint8_t *>(&config),
                                  offsetof(ConfigRecord, crc));
}

inline void configSetDefaults(ConfigRecord &config) {
  config.magic = ATQ_CONFIG_MAGIC;
  config.schema = ATQ_CONFIG_SCHEMA;
  config.left.rawMin = 0;
  config.left.rawMax = 4095;
  config.left.invert = 0;
  config.right.rawMin = 0;
  config.right.rawMax = 4095;
  config.right.invert = 0;
  config.filterPercent = 25;
  configSetCrc(config);
}

inline bool configRecordValid(const ConfigRecord &config) {
  return configValid(config) && configCrcValid(config);
}
