#pragma once

#include <stdlib.h>
#include "../util/atom.h"
#include "../peripheral/uart.h"
#include "../core/thread.h"

#define os_assert(cond, ...)                                    \
    atom {                                                      \
        if(!(cond)){                                            \
            Thread t = os_thisThread();                         \
            uart_printfUnlocked("Assertion failure @ %s:%d, "   \
                "Thread(@%p, name:\"%s\"):\n    ",              \
                __FILE__, __LINE__, t, os_threadName(t));       \
            uart_printfUnlocked(__VA_ARGS__);                   \
            uart_printfUnlocked("\n");                          \
            abort();                                            \
        }                                                       \
    }

