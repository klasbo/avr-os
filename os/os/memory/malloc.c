
#include <stdlib.h>
#include <string.h>

#include "../util/atom.h"
#include "../util/assert.h"
#include "../os_config.h"

__attribute__((malloc)) void* os_malloc(size_t len){
    void* r;
    atom r = malloc(len);
    os_assert(r, "Out of memory");
    return r;
}

void os_free(void* ptr){
    atom free(ptr);
}

void os_realloc(void *ptr, size_t len){
    atom realloc(ptr, len);
}

void malloc_init(void){
    // By default, malloc fails (returns NULL) if the allocation would collide with the stack
    // What malloc doesn't know, is that the "stack" is itself malloc'd
    // Disabling this "heap underflow" detection is fine, because we have stack overflow
    //  detection (though not for `main`).

    // Some PHP-level "equality" abuse requires some GCC abuse
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wundef"

    #if(HEAP_START != default)
        extern char* __malloc_heap_start;
        __malloc_heap_start = (char*)HEAP_START;
    #endif

    #if(HEAP_END == default)
        extern char* __malloc_heap_end;
        __malloc_heap_end = (char*)RAMEND;
    #else
        extern char* __malloc_heap_end;        
        __malloc_heap_end = (char*)HEAP_END;
    #endif

    #pragma GCC diagnostic pop
}