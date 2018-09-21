
#pragma once

#define MOVE_SP(stack)                                      \
    asm volatile (  "lds    r26, " #stack "         \n\t"   \
                    "lds    r27, " #stack " + 1     \n\t"   \
                    "out    __SP_L__, r26           \n\t"   \
                    "out    __SP_H__, r27           \n\t"   \
                );
