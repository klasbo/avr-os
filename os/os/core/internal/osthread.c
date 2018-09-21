
#include "osthread.h"

#include <string.h>

#include "../../memory/malloc.h"
#include "../../peripheral/uart.h"


OSThread* osthread_new(uint16_t stackSize){
    OSThread* t = os_malloc(sizeof(*t));
    memset(t, 0, sizeof(OSThread));
    
    if(stackSize != 0){
        t->stack.location   = os_malloc(stackSize);
        t->stack.size       = stackSize;
        t->stack.ptr        = (uint8_t*)(t->stack.location + stackSize - 1);
    } else {
        t->stack.location   = NULL;
        t->stack.size       = -1;
        t->stack.ptr        = NULL;
    }
    
    cdlist_reset(&t->schedList);
    cdlist_reset(&t->blockList);
    os_resource_init(&t->messageBox.resource);

    return t;
}


void osthread_delete(OSThread* t){
    if(t->stack.location){
        os_free(t->stack.location);
    }
    os_free(t);
}


uint8_t* osthread_initializeStack(uint8_t* stackPtr, void fn(void)){
    
    // Some known values, for debugging only
    *stackPtr-- = 0xab;
    *stackPtr-- = 0xcd;
    *stackPtr-- = 0xef;

    // Shove entry point on stack
    uint16_t fnAddr = (uint16_t)fn;
    *stackPtr-- = (uint8_t)(fnAddr      & (uint16_t)0x00ff);
    *stackPtr-- = (uint8_t)(fnAddr>>8   & (uint16_t)0x00ff);

    // Junk values for registers, as if saved by SAVE_CONTEXT
    *stackPtr-- = 0x00; // R0
    *stackPtr-- = 0x80; // SREG: Interrupts Enabled

    // R1..31
    for(uint8_t r = 1; r <= 31; r++){
        *stackPtr-- = 0x00;
    }

    return stackPtr;
}
