
#pragma once

#include "port/port.h"

#ifdef PORT_HAS_UART

    void uart_init(void);

    // `printf` over UART.
    //  Uses a Resource to allow only one thread at a time to use UART.
    void uart_printf(char const * fmt, ...) __attribute__((format(printf, 1, 2)));
    void uart_printf_P(char const * fmt, ...) __attribute__((format(printf, 1, 2)));

    // Like `uart_printf`, but without using the Resource.
    //  Intended only for debugging critical sections (as you cannot hold two resources at once)
    void uart_printfUnlocked(char const * fmt, ...) __attribute__((format(printf, 1, 2)));

    void uart_scanf(char const * fmt, ...) __attribute__((format(scanf, 1, 2)));
    void uart_scanf_P(char const * fmt, ...) __attribute__((format(scanf, 1, 2)));

#else
    #define unused __attribute__((unused))

    static inline __attribute__((deprecated("port does not have uart"))) 
        void uart_init(void){}

    static inline void uart_printf(char const * fmt unused, ...){}
    static inline void uart_printf_P(char const * fmt unused, ...){}
    static inline void uart_printfUnlocked(char const * fmt unused, ...){}

    static inline void uart_scanf(char const * fmt unused, ...){}
    static inline void uart_scanf_P(char const * fmt unused, ...){}

    #undef unused

#endif
