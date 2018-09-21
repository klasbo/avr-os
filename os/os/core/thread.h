
#pragma once

#include <stdint.h>

#include "time.h"


// A `Thread` is an opaque pointer type used to represent a unit of execution.
//  New `Thread`s are created by calling `os_spawn`.
//  Since `Thread` is an opaque pointer, it can be assigned to NULL, or functions
//   returning a `Thread` may return NULL.
//  Running threads are scheduled preemptively, and run in a random order.
typedef struct OSThread* Thread;

// Runs `fn` in a new thread.
//  `stackSize` is in number of bytes.
//  Be sure to allocate enough stack space, as functions like printf easily 
//   use ~30 bytes. Rudimentary stack overflow detection is implemented, but when
//   the stack overflows, the info it prints out could end up being garbage...
Thread os_spawn(void fn(void), uint16_t stackSize);

// Like `os_spawn`, but also gives the thread a human-readable name.
//  Intended only to ease debugging. The name of the currently executing thread 
//   is printed in all triggered assertions, and is also available by calling 
//   `os_threadName`.
Thread os_spawnNamed(void fn(void), uint16_t stackSize, char* name);


// Terminates the current thread. Call this function to "return" from a thread.
//  Calling `return` from a thread instead of calling `os_terminateThread` will
//   crash the OS, and should be considered a career-limiting move.
//  Calling this function in `main()` works just like with any other thread. If
//   `main()` has no loop (just spawns threads), then using `os_terminateThread`
//   is preferred over calling `os_sleep`, `os_receive`, or something similar.
//  Terminating a thread while it holds a resource will release the resource.
void os_terminateThread(void) __attribute__((noreturn));


// Returns the handle of the current thread.
Thread os_thisThread(void);


// Returns the handle of the thread that spawned the current thread.
//  Returns NULL if the owner has terminated, or if the current thread is `main()`.
Thread os_ownerThread(void);


// Returns the name of `t`, or NULL if it has no name.
//  If `t` is the thread representing `main()`, the returned value is "os_main".
char* os_threadName(Thread t);


// Puts the current thread to sleep for `dur` time.
//  The thread becomes runnable when the duration has passed. Note that 
//   it does not necessarily start running immediately if there are other 
//   runnable threads.
//  Calling this function with a duration of 0 is effectively the same as
//   calling `os_yield`.
void os_sleep(Time dur);


// Puts the current thread to sleep until `time`.
//  The thread becomes runnable at `time`. Note that it does not necessarily 
//   start running immediately if there are other runnable threads.
//  Calling this function with a time in the past will make this thread
//   runnable immediately.
void os_sleepUntil(Time time);


// Perform some action with a fixed period
// `period` must be of type `Duration`
// Example:
//      os_doPeriodically((Duration){.second = 1}){
//          PORTB ^= (1 << PB4);  // toggle led
//      }
#define os_doPeriodically(period)                                   \
    Time _w = os_time_now();                                        \
    for(;1;_w = os_time_add(_w, period), os_sleepUntil(_w))


// Force a context switch.
//  May be useful when polling external events with a while-loop, eg:
//      `while(!externalHardwareAvailable()){ os_yield(); }`
//  This will let other threads run for the remainder of the current tick.
//  Note that the next thread selected to run may be this thread. Do not rely
//   on `os_yield` to defer execution to "any other thread".
void os_yield(void);
