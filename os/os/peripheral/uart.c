
#include "../os_config.h"
#include "uart.h"
#include "port/port.h"
#include "../core/resource.h"
#include "../util/atom.h"

#ifdef PORT_HAS_UART

    #ifndef UART_BAUD
        #define UART_BAUD 38400
        #warning UART_BAUD defaults to 38400
    #endif
    

    static Resource uartResource;

    void uart_init(void){
        port_uart_init((uint16_t)(F_CPU/16/UART_BAUD - 1));
        os_resource_init(&uartResource);
        uartResource.name = "UART";
    }


    static FILE uart_stdio = FDEV_SETUP_STREAM(port_uart_putchar, port_uart_getchar, _FDEV_SETUP_RW);



    __attribute__((format(printf, 1, 2)))
    void uart_printf(char const * fmt, ...){
        //with(&uartResource){
            va_list v;
            va_start(v, fmt);
            vfprintf(&uart_stdio, fmt, v);
            va_end(v);
        //}
    }

    __attribute__((format(printf, 1, 2)))
    void uart_printf_P(char const * fmt, ...){
        with(&uartResource){
            va_list v;
            va_start(v, fmt);
            vfprintf_P(&uart_stdio, fmt, v);
            va_end(v);
        }
    }

    __attribute__((format(printf, 1, 2)))
    void uart_printfUnlocked(char const * fmt, ...){
        va_list v;
        va_start(v, fmt);
        vfprintf(&uart_stdio, fmt, v);
        va_end(v);
    }


    __attribute__((format(scanf, 1, 2)))
    void uart_scanf(char const * fmt, ...){
        with(&uartResource){
            va_list v;
            va_start(v, fmt);
            vfscanf(&uart_stdio, fmt, v);
            va_end(v);
        }
    }

    __attribute__((format(scanf, 1, 2)))
    void uart_scanf_P(char const * fmt, ...){
        with(&uartResource){
            va_list v;
            va_start(v, fmt);
            vfscanf_P(&uart_stdio, fmt, v);
            va_end(v);
        }
    }


#endif

