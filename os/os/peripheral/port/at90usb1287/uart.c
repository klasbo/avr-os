#if defined(__AVR_AT90USB1287__)


#include "uart.h"

#include <stdio.h>
#include <avr/io.h>


void port_uart_init(uint16_t baud){
    UBRR1 = baud;
    
    UCSR1B  |=  (1<<TXEN1) | (1<<RXEN1);
    
    UCSR1C  |=  (3 << UCSZ10);   // char size to 8
}

void port_uart_putchar(char c){
    while( !(UCSR1A & (1<<UDRE1)) ){}
    
    UDR1 = c;
}

char port_uart_getchar(void){
    while( !(UCSR1A & (1<<RXC1)) ){}

    return UDR1;
}

#endif


