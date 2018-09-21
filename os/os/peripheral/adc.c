
#include "adc.h"

#include <avr/io.h>

#include "../core/resource.h"

#ifdef PORT_HAS_ADC

    static Resource adcResource;

    void adc_init(void){    
        port_adc_init();
        os_resource_init(&adcResource);
        adcResource.name = "ADC";
    }


    uint16_t adc_read(uint8_t mux){
        uint16_t v;
        with(&adcResource){
            v = port_adc_read(mux);
        }
        return v;
    }

#endif