#if defined(__AVR_AT90USB1287__)

#include "adc.h"

#include <avr/io.h>


void port_adc_init(void){    
    ADMUX = 0;

    ADCSRA  |=  (1<<ADEN)
            |   (1<<ADPS2)  // Clock prescaler to 128x
            |   (1<<ADPS1)
            |   (1<<ADPS0);
}


uint16_t port_adc_read(uint8_t mux){
    ADMUX = mux;
    ADCSRA |= (1<<ADSC);
    while( (ADCSRA & (1<<ADSC)) ){}
    return ADC;
}

#endif