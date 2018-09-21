
#include "led.h"

#include <avr/io.h>

#include "os/os.h"

void led_init(void){
    DDRB    |=  (1<<DDB0)
            |   (1<<DDB2)
            |   (1<<DDB4)
            |   (1<<DDB6);
    PORTB   |=  (1<<PB0)
            |   (1<<PB2)
            |   (1<<PB4)
            |   (1<<PB6);
}

void led_set(int8_t led_id, int8_t value){
    if(led_id < 0 || led_id > 3){
        return;
    }
    
    atom {
        if(value){
            PORTB &= ~(1 << (led_id*2));
        } else {
            PORTB |= (1 << (led_id*2));
        }
    }
}