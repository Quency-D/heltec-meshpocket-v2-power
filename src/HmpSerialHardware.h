#pragma once
#include <stdint.h>
extern "C" {
void hmp_v2_uart_start(void *serial, uint8_t rx, uint8_t tx, uint32_t baud);
int32_t hmp_v2_uart_available(void *serial);
int32_t hmp_v2_uart_read(void *serial);
void hmp_v2_output(uint8_t pin);
void hmp_v2_high(uint8_t pin);
void hmp_v2_pulldown(uint8_t pin);
uint8_t hmp_v2_pin_high(uint8_t pin);
uint32_t hmp_v2_millis();
void hmp_v2_irq_off();
void hmp_v2_irq_on();
}
