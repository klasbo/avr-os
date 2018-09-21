
#pragma once

#include <stdint.h>

#define PORT_HAS_UART

void port_uart_init(uint16_t baud);
void port_uart_putchar(char c);
char port_uart_getchar(void);

