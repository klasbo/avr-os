#pragma once

#include <util/atomic.h>

/* Create an atomic block. Anything inside this block will not be interrupted.
Example:
atom {
    // Critical section
}
*/
#define atom ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

