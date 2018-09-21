


#include "led.h"
#include "switch.h"

#include "os/os.h"
#include "os/examples.h"


#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>


void blink1(void){
    while(1){
        os_sleep((Time){.msecs = 750});
        uart_printf("Hello from thread 1\n");
    }
}

void blink2(void){
    while(1){
        os_sleep((Time){.seconds = 1});
        uart_printf("Hello from thread 2\n");
    }
}



int main(void){
    led_init();
    uart_init();
        
    os_spawn(blink1, 100);
    os_spawn(blink2, 100);

    while(1){
        os_sleep((Time){.hours = 1});
    }

}


