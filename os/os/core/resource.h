#pragma once

#include "../structure/cdlist.h"


// A `Resource` is a way of controlling access to shared data/peripherals/etc.
//
//  A `Resource` can only be held by one thread at a time. Once a thread `t` 
//   has acquired a particular resource, any other threads that try to acquire 
//   it will be blocked until `t` releases the resource. The next thread to get 
//   the resource is the first one that was blocked (FIFO).
//  A thread can only acquire one `Resource` at a time. If a thread tries to 
//   acquire a second resource, an assert will be triggered.
//  A thread can not send a message while holding a resource. This is because 
//   the message box of a thread is itself a resource. An assert will be 
//   triggered if a thread sends a message while holding a resource.
//  Resources can optionally be given names, to ease debugging. The names of the
//   resources are printed by the aforementioned asserts (the resource held, and
//   the resource attempted to be acquired). 
//
// Rationale for allowing only one resource at a time:
//  - This is a message-passing OS, so discouraging shared variables is an 
//    ideological position. If you feel like you need to hold multiple resources, 
//    you should either a) think more in terms of messages ("Don't communicate by 
//    sharing memory, share memory by communicating"), or b) use another (more 
//    "traditional") OS.
//  - Resources should represent something that is "shared" by nature, and 
//    sharing is only possible if you don't hold on to something for too long. 
//    If you're holding a resource while doing something that will potentially 
//    block (like sending a message, or waiting for another resource), then the 
//    program will eventually (and inevitably, in my experience) end up threads 
//    not getting access to the resources they need: a deadlock. Designing the 
//    API so that deadlocks are impossible is (probably) not feasible in C, but 
//    at least allowing only one resource at a time is a step in a direction some 
//    would consider "right".
//  - Traditional mutexes compose poorly, mostly because shared variables are 
//    "bad" for the same reasons global variables are (ie. impossible to say what, 
//    when and by whom something is modified), but now with the added complexity 
//    of remembering to lock the mutex in the first place. Forget to use a mutex, 
//    and you have a race condition. But if you use them too much, you have a 
//    deadlock. The mental overhead of shared/global variables combined with shared 
//    locks easily becomes too much.
typedef struct Resource Resource;
struct Resource {
    CDListItem  handle;
    char*       name;
};

// Initialize a resource.
//  Acquiring an uninitialized resource will crash the OS, so this function 
//   must be called before using the resource.
//  Normally, you would only need to initialize a resource once. Re-initializing
//   a resource while it is in use may cause the OS to crash.
//  Initializing a resource will reset its name.
void os_resource_init(Resource* r);

// Acquire a resource.
//  It is only possible to acquire one resource at a time.
void os_resource_acquire(Resource* r);

// Release a resource.
void os_resource_release(Resource* r);

// Create a scope where `resource` is automatically acquired and released.
//  Example:
//      Resource r;
//      os_resource_init(&r);
//      with(&r){
//          // Critical section: No other threads can have acquired `r`
//      }
//      // `r` is released at this point
//
//  Prefer using `with` instead of acquire/release, as it's impossible to 
//  forget to release the resource. Even calling `return` or `break` inside 
//  this statement will release the resource.
#define with(resource)                                                                                                                             \
    for(                                                        /* --- Comma-operator-abuse explained: --- //                                   */ \
        Resource                                                /* Create two Resource* vars: (multi-var for-loop must have vars of same type)  */ \
            *_r1 = (Resource*)1,                                /*  _r1 is just a dummy                                                         */ \
            __attribute__((cleanup(_res_relptr))) *_r2 =        /*  _r2 is hooked to cleanup                                                    */ \
                (os_resource_acquire(resource), resource);      /*  _r2 initialized to `resource`, after it has been acquired                   */ \
        _r1;                                                    /* Check value of dummy pointer to terminate loop, which will be NULL when...   */ \
        _r1 = NULL                                              /*  we assign NULL to it after the loop body is executed                        */ \
    )

// (Internal)
static inline void _res_relptr(Resource** r){
    os_resource_release(*r);
}