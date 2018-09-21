Peripherals
===========

Peripherals are all things that are platform-specific (or at least require platform-specific initialization), and are not thread-safe by default. An example would be an ADC: One thread could start a conversion while a conversion already is in progress, or two threads could be polling the same "conversion done" flag, leading to one of them getting a wring result.

The structure and layout of both the folders and code is an attempt to
 - provide a layer of thread-safe abstraction over the raw hardware, where only the thread-safe layer is `#include`d by the user
 - allow for extending the set of peripherals and set of platforms independently, with sensible error messages when using a peripheral that does not exist (or is not implemented) for that platform, and no linker errors due to duplicate function definitions (one for each platform)

Porting to a new platform
-------------------------

A port for a particular peripheral `periph` for platform `platf` should comply with the following:
 1) The platform-specific code should be in a source/header pair called `port/platf/periph.{c,h}` (ie. in the correct folder)
 2) The code in `port/platf/periph.{c,h}` should not have calls to any os functions
 3) The source in `port/platf/periph.c` should be in a `#if defined(__AVR_platf__) [...] #endif` block (This is to prevent linker conflicts)
 4) The header `port/platf/periph.h` should `#define PORT_HAS_PERIPH`, to state that the peripheral is available
 5) `port/port.h` should be updated to `#include "platf/periph.h"`, added to the list of includes in the correct `#if defined(__AVR_platf__)` block
 6) Ports for existing peripherals should define the following functions (please add to this list as new peripherals are added):
    - UART:
      - `void port_uart_init(uint16_t baud)`
      - `void port_uart_putchar(char c)`
      - `char port_uart_getchar(void)`
    - ADC:
      - `void port_adc_init(void)`
      - `uint16_t port_adc_read(uint8_t mux)`
      
      
Adding a new peripheral
-----------------------

The thread-safe abstraction for a particular peripheral `periph` should comply with the following:
 1) The code should be in a source/header pair called `periph.{c,h}`
 2) `periph.h` should `#include "port/port.h"`.
 3) The function declarations in `periph.h` should be in a `#if PORT_HAS_PERIPH [...] #endif` block. (Alternatively a `#if PORT_HAS_PERIPH [...] #else [...] #endif` block can be used, where the `#else` block provides body-less declarations of the type `static inline __attribute__((deprecated("port does not have periph")))`)
 4) A single `Resource` should be used for critical sections.
 5) Any extended busy-waiting/polling of the type `while(expr){}` should be replaced with `while(expr){ os_yield(); }`
