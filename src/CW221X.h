#ifndef HELTEC_MESHPOCKET_V2_POWER_PUBLIC_CW221X_H
#define HELTEC_MESHPOCKET_V2_POWER_PUBLIC_CW221X_H

#if defined(HMP_BUILD_CORE)
class TwoWire;
extern TwoWire Wire;
#else
#include <HeltecMeshPocketV2PowerTarget.h>
#include <Wire.h>
#endif
#include <stddef.h>
#include <stdint.h>

class CW221X {
public:
  enum Error : uint8_t {
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

static_assert(sizeof(bool) == 1 && alignof(CW221X::Status) == 2 &&
              sizeof(CW221X::Status) == 10 && offsetof(CW221X::Status, voltageMv) == 4 &&
              offsetof(CW221X::Status, profile) == 8, "Unsupported gauge status layout");
#endif
