#include "HmpSerialHardware.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#if (defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_NRF52_ADAFRUIT) ||        \
     defined(NRF52_SERIES)) &&                                                \
    __has_include(<Uart.h>)
#include <Uart.h>
#define HLT331_HAS_NRF52_UART 1
#endif
void hmp_v2_uart_start(void *port, uint8_t rx, uint8_t tx, uint32_t baud) {
  auto &serial = *static_cast<HardwareSerial *>(port);
#if defined(HLT331_HAS_NRF52_UART)
  static_cast<Uart &>(serial).setPins(rx, tx);
#else
  (void)rx;
  (void)tx;
#endif
  serial.begin(baud);
}
int32_t hmp_v2_uart_available(void *p) { return static_cast<HardwareSerial *>(p)->available(); }
int32_t hmp_v2_uart_read(void *p) { return static_cast<HardwareSerial *>(p)->read(); }
void hmp_v2_output(uint8_t pin) { pinMode(pin, OUTPUT); }
void hmp_v2_high(uint8_t pin) { digitalWrite(pin, HIGH); }
void hmp_v2_pulldown(uint8_t pin) { pinMode(pin, INPUT_PULLDOWN); }
uint8_t hmp_v2_pin_high(uint8_t pin) { return digitalRead(pin) == HIGH; }
uint32_t hmp_v2_millis() { return millis(); }
void hmp_v2_irq_off() { noInterrupts(); }
void hmp_v2_irq_on() { interrupts(); }
