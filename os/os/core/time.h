
#pragma once

#include <stdint.h>

// The `Time` struct is used to represent time, both in the sense of "the 
//  current time" and "a duration of time".
typedef struct Time Time;
struct Time {
    uint16_t    msecs;
    uint8_t     seconds;
    uint8_t     minutes;
    uint8_t     hours;
};


// Return the time since the OS was started.
Time os_time_now(void);


// Sum two `Time`s.
Time os_time_add(Time t1, Time t2);


// Subtract two `Time`s.
//  Since negative times can not be represented, an assert will be triggered
//  if `t1 < t2`. See also `os_time_difference`.
Time os_time_subtract(Time t1, Time t2);


// Subtract the smaller of the two parameters from the larger.
// Example:
//      Time t1 = os_time_difference((Time){.seconds = 1}, (Time){.seconds = 4});
//      Time t2 = os_time_difference((Time){.seconds = 4}, (Time){.seconds = 1});
//      os_assert(t1.seconds == 3, " ");
//      os_assert(os_time_cmp(t1, t2) == 0, " ");
Time os_time_difference(Time t1, Time t2);


// Compare two `Time`s.
// Returns:
//   1  if  t1 > t2
//   0  if  t1 == t2
//  -1  if  t1 < t2
int8_t os_time_cmp(Time t1, Time t2);


// Convenience macros for printing `Time`s.
// Example:
//      Time t = (Time){.hours = 1, .minutes = 5, .seconds = 48, .msecs = 2};
//      uart_printf("t: " os_time_fmtStr "\n", os_time_fmtParam(t));
//      // prints "t: Time(1h, 5m, 48s, 2t)"
#define os_time_fmtStr "Time(%dh, %dm, %ds, %dt)"
#define os_time_fmtParam(t) t.hours, t.minutes, t.seconds, t.msecs