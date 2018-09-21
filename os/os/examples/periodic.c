#include "../os.h"

/*
This example shows how to perform periodic task execution.

Jitter is unavoidable, as threads do not have priorities. Note that jitter
 is not necessarily in whole ticks, as the previously running thread may have 
 called `os_yield` explicitly (forcing a context switch before the next OS msecs).

If the period is too short, there will inevitably be overruns. A sleep for a 
 duration of 0 ticks is effectively the same as a call to `os_yield`.

Using os_sleepUntil will mitigate the drift you would get from just calling 
 `os_sleep` with a fixed duration.
*/

// Manual/verbose/explicit/flexible (take your pick) periodic execution
// This example is mostly for illustrating what goes on "behind the scenes" 
//  of `os_doPeriodically`, which you should probably prefer using (see below).
// If your application needs a varying period, `os_doPeriodically` will 
//  obviously not work.
static void periodicFn1(void){
    Time period = (Time){.seconds = 1, .msecs = TICK_HZ/4};
    Time wakeTime = os_time_now();
    while(1){
        // Perform the periodic action
        Time now = os_time_now();
        uart_printf("Fn1: now: " os_time_fmtStr "\n", os_time_fmtParam(now));

        // Add the period to the next wake time, and sleep
        wakeTime = os_time_add(wakeTime, period);
        os_sleepUntil(wakeTime);
    }
}

// Concise and to the point, and arguably also more readable
static void periodicFn2(void){
    os_doPeriodically((Time){.seconds = 1}){
        Time now = os_time_now();
        uart_printf("Fn2: now: " os_time_fmtStr "\n", os_time_fmtParam(now));
    }
}

void examples_periodic(void){
    os_spawn(periodicFn1, 100);
    os_spawn(periodicFn2, 100);
}

