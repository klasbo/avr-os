
#include "../structure/cdlist.h"
#include "../util/assert.h"
#include "internal/osthread.h"
#include "internal/sched.h"

void os_resource_init(Resource* r){
    cdlist_reset(&r->handle);
    r->name = NULL;
}

void os_resource_acquire(Resource* r){
    OSThread* t = sched_currentThread();

    os_assert(cdlist_empty(&t->blockList),
        "Cannot block on multiple resources (has %p:\"%s\", tried to get %p:\"%s\").\n",
        enclosing(t->blockList.prev, Resource, handle), enclosing(t->blockList.prev, Resource, handle)->name ?: "", 
        r, r->name ?: "");

    /*
    New entries to resource-list can only be "behind" us
      after we have checked if we are first, no new entries can be ahead of us
      ok to unlock atom after this check (no double locking needed)
    A thread may release the resource after we check if it is available
      the resource must not become available before we set ourselves to blocked 
      => changing sched state must be in the same atom
    */

    // Cannot os_yield inside an atom: 
    //  check performed inside atom, os_yield performed outside.
    //  double yield (in case of yield from tick) has no ill effect.
    uint8_t resourceAvailable = 0;
    atom {
        cdlist_append(&r->handle, &t->blockList);
        resourceAvailable = (cdlist_idx(&r->handle, 0) == &t->blockList);
        if(!resourceAvailable){
            sched_state_setBlockedResource(t);
        }
    }

    if(!resourceAvailable){
        sched_yield();
        // Thread should only become runnable once we have the resource
    }
}

void os_resource_release(Resource* r){
    OSThread* t = sched_currentThread();

    atom {
        cdlist_unlink(&t->blockList);
        cdlist_reset(&t->blockList);
        if(cdlist_length(&r->handle) > 0){
            OSThread* nextInLine = enclosing(cdlist_idx(&r->handle, 0), OSThread, blockList);
            sched_state_setRunnable(nextInLine);
        }
    }
}