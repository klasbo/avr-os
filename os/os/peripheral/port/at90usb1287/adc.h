#pragma once

#include <stdint.h>

#define PORT_HAS_ADC

void port_adc_init(void);
uint16_t port_adc_read(uint8_t mux);

