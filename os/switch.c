
#include "switch.h"

#include <avr/io.h>

#include "os/os.h"

void switch_init(void){
    DDRB    &=  ~(1<<DDB1);
    DDRB    &=  ~(1<<DDB3);
    DDRB    &=  ~(1<<DDB5);
    DDRB    &=  ~(1<<DDB7);
}

int8_t switch_read(int8_t switch_id){
    if(switch_id < 0 || switch_id > 3){
        return 0;
    }
    uint8_t v;
    atom {
        v = !( PINB & (1 << (switch_id*2 + 1)) );
    }
    return v;
}