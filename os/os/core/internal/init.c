
#include "../../memory/malloc_private.h"
#include "sched.h"


__attribute__((constructor)) void os_init(void){
    malloc_init();
    sched_init();
}