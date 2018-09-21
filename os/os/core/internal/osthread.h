
#pragma once

#include "../../core/resource.h"
#include "../../core/time.h"
#include "../../structure/cdlist.h"

#include <stdint.h>

typedef struct OSThread OSThread;
struct OSThread {
    struct {
        uint8_t*        ptr;        // Must be first member!
        uint8_t*        location;   // For stack overflow detection & free
        uint16_t        size;       // For stack overflow detection
    }               stack;

    CDListItem      schedList;
    CDListItem      blockList;
    Time            wakeTime;

    struct {
        Resource        resource;
        uint16_t        typeId;
        void*           addr;
        uint8_t         unreadMsg;
    }               messageBox;

    OSThread*       owner;
    /*
    SList           spawn;
    SListItem       spawnLink;
    */
    char*           name;
} __attribute__((packed));

/*
when this spawns that:
    slist_append(this.spawn, that.spawnLink);
when this terminates:
    slist_foreach(item, this.spawn){
        OSThread* spawnedThread = enclosing(item, OSThread, spawnLink);
        slist_reset(&spawnedThread->spawnLink);
        spawnedThread->owner = NULL;
    }
*/

// Just creates a new thread with fields initialized. Does not schedule it.
OSThread* osthread_new(uint16_t stackSize);

// Just deletes the memory used by this thread
//  If the thread that calls this function is `t`, then the calls to `os_free` 
//   will be run on the stack that is being freed. Moving the stack pointer 
//   before calling this function is probably "most correct", but it seems to 
//   "work fine" without: The freelist item in malloc is put at the "top" of the 
//   memory region, but the stack grows from the "bottom", and we have ~40 bytes 
//   (extra for saving the context) free on the top.
void osthread_delete(OSThread* t);

// Put entry point and empty registers on stack
uint8_t* osthread_initializeStack(uint8_t* stackPtr, void fn(void));

