#include <Arduino.h>
#include <Wire.h>

extern "C" {
void hmp_v2_wire_begin(void *p) { static_cast<TwoWire *>(p)->begin(); }
void hmp_v2_wire_clock(void *p, uint32_t hz) { static_cast<TwoWire *>(p)->setClock(hz); }
void hmp_v2_wire_start(void *p, uint8_t addr) { static_cast<TwoWire *>(p)->beginTransmission(addr); }
uint32_t hmp_v2_wire_byte(void *p, uint8_t byte) { return static_cast<TwoWire *>(p)->write(byte); }
uint32_t hmp_v2_wire_bytes(void *p, const uint8_t *data, uint8_t len) {
  return static_cast<TwoWire *>(p)->write(data, len);
}
uint8_t hmp_v2_wire_end(void *p, uint8_t stop) { return static_cast<TwoWire *>(p)->endTransmission(stop); }
uint8_t hmp_v2_wire_request(void *p, uint8_t addr, uint8_t len) {
  return static_cast<TwoWire *>(p)->requestFrom(addr, len);
}
int32_t hmp_v2_wire_available(void *p) { return static_cast<TwoWire *>(p)->available(); }
int32_t hmp_v2_wire_receive(void *p) { return static_cast<TwoWire *>(p)->read(); }
void hmp_v2_delay(uint32_t ms) { delay(ms); }
}
