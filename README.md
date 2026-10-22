# HeltecMeshPocketV2Power

Precompiled CW221X fuel-gauge and HLT331 power-status library for Heltec
Mesh Pocket V2. Version `0.1.0`; either module can be used independently.

## Requirements

- Meshtastic `heltec-mesh-pocket-v2`: nRF52840, Cortex-M4, hard-float, C++17.
- PlatformIO `nordicnrf52@11.0.0` and ARM GCC 12.3.1.

Keep the supplied headers and `.a` together when updating the library.

## Usage

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
IO pins with `configureIoPins()`, and call `poll()` regularly before
`readStatus(status)`. Check its return value before using measurements, then
check `status.ageMs` for freshness. The snapshot includes ports, charging
modes, voltage, current, `ntc1TemperatureC`, `ntc2TemperatureC`, and IO state.
See [HLT331Serial.h](src/HLT331Serial.h) for the full interface.

## Examples

- [MeshPocketV2Basic](examples/MeshPocketV2Basic/MeshPocketV2Basic.ino): CW221X readings on the TFT.
- [MeshPocketV2CombinedMonitor](examples/MeshPocketV2CombinedMonitor/MeshPocketV2CombinedMonitor.ino): CW221X and HLT331 monitor.

Both TFT examples require the matching Mesh Pocket V2 board variant and
`Arduino_GFX_Library`.

Maintainer: Quency-D. CW221X driver origin: Cellwise.
