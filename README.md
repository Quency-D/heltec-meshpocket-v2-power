# HeltecMeshPocketV2Power

Precompiled CW221X fuel-gauge and HLT331 power-status library for Heltec
Mesh Pocket V2. Version `0.2.0`; either module can be used independently.

## Requirements

nRF52840 Cortex-M4, hard-float Arduino target, C++11 or later. Release compatibility checks cover C++11, C++14, C++17 and C++20.

## Usage

Add the same ZIP to either environment's existing `lib_deps` list:

```ini
lib_deps =
  https://github.com/Quency-D/heltec-meshpocket-v2-power/releases/download/v0.2.0/HeltecMeshPocketV2Power-0.2.0.zip
```

Both projects compile the Arduino adapters locally and link the same archive.

Include `<CW221X.h>` and `<Wire.h>` for the fuel gauge, or
`<HLT331Serial.h>` for the power-status monitor.

## Interfaces

**CW221X:** call `begin(Wire)`, then `readStatus(status)` to update a
`CW221X::Status`. Check the return value before using measurements;
`lastError()` reports the driver error. Failed reads can leave partially
updated fields.

| Status field | Meaning |
|---|---|
| `ready`, `error` | Read result and error code |
| `socPercent` | Battery charge, capped at 100% |
| `voltageMv` | Battery voltage in mV |
| `temperatureX10C` | Signed temperature in 0.1 °C |
| `profile.matches` | Battery-profile match used for fault and low-SOC protection |

**HLT331Serial:** call `begin(Serial1, PIN_9006_RX, PIN_9006_TX)`, configure
IO pins with `configureIoPins(io1InterruptPin, io2WorkPin)`, and call `poll()` regularly before
`readStatus(status)`. Check its return value before using measurements, then
check `status.ageMs` for freshness. The snapshot includes ports, charging
modes, voltage, current, `ntc1TemperatureC`, `ntc2TemperatureC`, and IO state.
See [HLT331Serial.h](src/HLT331Serial.h) for the full interface.

## Examples

- [MeshPocketV2Basic](examples/MeshPocketV2Basic/MeshPocketV2Basic.ino): CW221X readings on the TFT.
- [MeshPocketV2CombinedMonitor](examples/MeshPocketV2CombinedMonitor/MeshPocketV2CombinedMonitor.ino): CW221X and HLT331 monitor.

Both TFT examples require the matching Mesh Pocket V2 board variant and
`Arduino_GFX_Library`.

No whole-library open-source license is provided.

Maintainer: Quency-D. CW221X driver origin: Cellwise.
