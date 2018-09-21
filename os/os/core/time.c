
#include "time.h"

#include "../os_config.h"
#include "../util/assert.h"

static Time now = {0};


static void os_time_normalize(Time* t){
    if(t->msecs >= TICK_HZ){
        t->seconds += t->msecs/TICK_HZ;
        t->msecs = t->msecs % TICK_HZ;
    }

    if(t->seconds >= 60){
        t->minutes++;
        t->seconds = t->seconds % 60;
    }

    if(t->minutes >= 60){
        t->hours++;
        t->minutes = t->minutes % 60;
    }
}

void time_incrementTick(void){
    now.msecs++;
    if(now.msecs >= TICK_HZ){
        os_time_normalize(&now);
    }
}



Time os_time_now(void){
    return now;
}


Time os_time_add(Time t1, Time t2){
    Time result = (Time){
        .msecs   = t1.msecs    + t2.msecs,
        .seconds = t1.seconds  + t2.seconds,
        .minutes = t1.minutes  + t2.minutes,
        .hours   = t1.hours    + t2.hours,
    };

    os_time_normalize(&result);

    return result;
}


static Time os_time_subtractImpl(Time t1, Time t2){

    Time result = {0};

    result.hours = t1.hours - t2.hours;

    #define subAndCarry(low, high, mult)      \
        if(t1.low < t2.low){            \
            result.high--;              \
            t1.low += mult;             \
        }                               \
        result.low = t1.low - t2.low;

    subAndCarry(minutes,    hours,      60)
    subAndCarry(seconds,    minutes,    60)
    subAndCarry(msecs,      seconds,    TICK_HZ)

    #undef carry

    return result;
}


Time os_time_subtract(Time t1, Time t2){
    os_assert(os_time_cmp(t1, t2) > 0, 
        "Cannot subtract times: t1:"os_time_fmtStr" < t2:"os_time_fmtStr"\n", 
        os_time_fmtParam(t1), os_time_fmtParam(t2));

    return os_time_subtractImpl(t1, t2);
}


Time os_time_difference(Time t1, Time t2){
    if(os_time_cmp(t1, t2) < 0){
        return os_time_subtractImpl(t2, t1);
    } else {
        return os_time_subtractImpl(t1, t2);
    }
}




int8_t os_time_cmp(Time t1, Time t2){
    #define cmp(field) \
        if(t1.field > t2.field) return 1; \
        if(t1.field < t2.field) return -1;

    cmp(hours)
    cmp(minutes)
    cmp(seconds)
    cmp(msecs)
    return 0;

    #undef cmp
}



void __attribute__((constructor)) _os_time_test(void){
    Time t1 = (Time){.hours = 1};
    Time t2 = (Time){.minutes = 59, .seconds = 59, .msecs = TICK_HZ-1};
    Time t3 = os_time_subtract(t1, t2);
    os_assert(!os_time_cmp(t3, (Time){.msecs = 1}), " ");
    os_assert(!os_time_cmp(os_time_add(t2, t3), t1), " ");
    os_assert(!os_time_cmp(os_time_add(t3, t2), t1), " ");
    Time t4 = os_time_difference(t2, t1);
    os_assert(!os_time_cmp(t3, t4), " ");
}