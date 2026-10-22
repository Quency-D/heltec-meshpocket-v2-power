#ifndef HELTEC_MESHPOCKET_V2_POWER_PUBLIC_CW221X_H
#define HELTEC_MESHPOCKET_V2_POWER_PUBLIC_CW221X_H

#include <HeltecMeshPocketV2PowerTarget.h>
#include <Wire.h>
#include <stdint.h>

class CW221X {
public:
  enum Error {
    ERROR_NONE = 0,
    ERROR_NOT_BEGUN = 1,
    ERROR_I2C = 2,
    ERROR_CHIP_ID = 3,
    ERROR_INVALID_ARGUMENT = 4,
    ERROR_TIMEOUT = 5
  };

  struct ProfileCheck {
    bool matches = false;
  };

  struct Status {
    bool ready = false;
    Error error = ERROR_NONE;
    uint8_t socPercent = 0;
    uint16_t voltageMv = 0;
    int16_t temperatureX10C = 0;
    ProfileCheck profile{};
  };

  CW221X();
  ~CW221X();
  CW221X(const CW221X &) = delete;
  CW221X &operator=(const CW221X &) = delete;
  CW221X(CW221X &&) = delete;
  CW221X &operator=(CW221X &&) = delete;

  bool begin(TwoWire &wire = Wire);
  bool readStatus(Status &status);
  Error lastError() const;

private:
  alignas(8) unsigned char storage_[128];
};

#endif
