#pragma once

#include <stdint.h>
#include "port/port.h"

#ifdef PORT_HAS_ADC

    void adc_init(void);

    uint16_t adc_read(uint8_t mux);

#else

    #define unused __attribute__((unused))

    static inline __attribute__((deprecated("port does not have adc"))) 
        void adc_init(void){}

    static inline uint16_t adc_read(uint8_t mux unused){ return 0; }

    #undef unused

#endif

