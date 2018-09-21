
#include "sched.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <stdint.h>

#include "../../os_config.h"

#include "../../core/time_private.h"
#include "../../memory/malloc.h"
#include "../../util/assert.h"
#include "../../util/atom.h"
#include "../../util/random.h"
#include "move_sp.h"
#include "osthread.h"


static volatile     OSThread*       currentThread;
static              uint32_t        threadSelectRandCtx;

static              OSThread*       idleThread;

static              void*           ctxSwitchStack;

static              CDListItem      list_runnable;
static              CDListItem      list_sleeping;
static              CDListItem      list_blockedResource;
//static              CDListItem      list_blockedInterrupt; // TODO
static              CDListItem      list_blockedRecv;
static              CDListItem      list_blockedSend;


#define schedLists_foreach(identifier, statement)                       \
    {                                                                   \
        CDListItem* _lists[] = {                                        \
            &list_runnable,                                             \
            &list_sleeping,                                             \
            &list_blockedResource,                                      \
            /*&list_blockedInterrupt,*/                                     \
            &list_blockedRecv,                                          \
            &list_blockedSend                                           \
        };                                                              \
                                                                        \
        for(uint8_t _n = 0;                                             \
            _n < (sizeof(_lists)/sizeof(_lists[0]));                    \
            _n++                                                        \
        ){                                                              \
            CDListItem* identifier = _lists[_n];                        \
            statement                                                   \
        }                                                               \
    }
    
#define threads_foreach(identifier, statement)                          \
    schedLists_foreach(_list,                                           \
        cdlist_foreach(_item, _list){                                   \
            OSThread* identifier = enclosing(_item, OSThread, schedList);   \
            statement                                                   \
        }                                                               \
    )




#define SAVE_CONTEXT()                                          \
    asm volatile (  "push   r0                          \n\t"   \
                    "in     r0, __SREG__                \n\t"   \
                    "cli                                \n\t"   \
                    "push   r0                          \n\t"   \
                    "push   r1                          \n\t"   \
                    "clr    r1                          \n\t"   \
                    "push   r2                          \n\t"   \
                    "push   r3                          \n\t"   \
                    "push   r4                          \n\t"   \
                    "push   r5                          \n\t"   \
                    "push   r6                          \n\t"   \
                    "push   r7                          \n\t"   \
                    "push   r8                          \n\t"   \
                    "push   r9                          \n\t"   \
                    "push   r10                         \n\t"   \
                    "push   r11                         \n\t"   \
                    "push   r12                         \n\t"   \
                    "push   r13                         \n\t"   \
                    "push   r14                         \n\t"   \
                    "push   r15                         \n\t"   \
                    "push   r16                         \n\t"   \
                    "push   r17                         \n\t"   \
                    "push   r18                         \n\t"   \
                    "push   r19                         \n\t"   \
                    "push   r20                         \n\t"   \
                    "push   r21                         \n\t"   \
                    "push   r22                         \n\t"   \
                    "push   r23                         \n\t"   \
                    "push   r24                         \n\t"   \
                    "push   r25                         \n\t"   \
                    "push   r26                         \n\t"   \
                    "push   r27                         \n\t"   \
                    "push   r28                         \n\t"   \
                    "push   r29                         \n\t"   \
                    "push   r30                         \n\t"   \
                    "push   r31                         \n\t"   \
                    "lds    r26, currentThread          \n\t"   \
                    "lds    r27, currentThread + 1      \n\t"   \
                    "in     r0, __SP_L__                \n\t"   \
                    "st     x+, r0                      \n\t"   \
                    "in     r0, __SP_H__                \n\t"   \
                    "st     x+, r0                      \n\t"   \
                );


#define RESTORE_CONTEXT()                                       \
    asm volatile (  "lds    r26, currentThread          \n\t"   \
                    "lds    r27, currentThread + 1      \n\t"   \
                    "ld     r28, x+                     \n\t"   \
                    "out    __SP_L__, r28               \n\t"   \
                    "ld     r29, x+                     \n\t"   \
                    "out    __SP_H__, r29               \n\t"   \
                    "pop    r31                         \n\t"   \
                    "pop    r30                         \n\t"   \
                    "pop    r29                         \n\t"   \
                    "pop    r28                         \n\t"   \
                    "pop    r27                         \n\t"   \
                    "pop    r26                         \n\t"   \
                    "pop    r25                         \n\t"   \
                    "pop    r24                         \n\t"   \
                    "pop    r23                         \n\t"   \
                    "pop    r22                         \n\t"   \
                    "pop    r21                         \n\t"   \
                    "pop    r20                         \n\t"   \
                    "pop    r19                         \n\t"   \
                    "pop    r18                         \n\t"   \
                    "pop    r17                         \n\t"   \
                    "pop    r16                         \n\t"   \
                    "pop    r15                         \n\t"   \
                    "pop    r14                         \n\t"   \
                    "pop    r13                         \n\t"   \
                    "pop    r12                         \n\t"   \
                    "pop    r11                         \n\t"   \
                    "pop    r10                         \n\t"   \
                    "pop    r9                          \n\t"   \
                    "pop    r8                          \n\t"   \
                    "pop    r7                          \n\t"   \
                    "pop    r6                          \n\t"   \
                    "pop    r5                          \n\t"   \
                    "pop    r4                          \n\t"   \
                    "pop    r3                          \n\t"   \
                    "pop    r2                          \n\t"   \
                    "pop    r1                          \n\t"   \
                    "pop    r0                          \n\t"   \
                    "out    __SREG__, r0                \n\t"   \
                    "pop    r0                          \n\t"   \
                );





void sched_state_clear(OSThread* t){
    cdlist_unlink(&t->schedList);
    cdlist_reset(&t->schedList);
}

void sched_state_setRunnable(OSThread* t){
    sched_state_clear(t);
    cdlist_append(&list_runnable, &t->schedList);
}

void sched_state_setSleeping(OSThread* t){
    sched_state_clear(t);
    cdlist_append(&list_sleeping, &t->schedList);
}

void sched_state_setBlockedResource(OSThread* t){
    sched_state_clear(t);
    cdlist_append(&list_blockedResource, &t->schedList);
}

/*void sched_state_setBlockedInterrupt(OSThread* t){
    sched_state_clear(t);
    cdlist_append(&list_blockedInterrupt, &t->schedList);
}*/

void sched_state_setBlockedRecv(OSThread* t){
    sched_state_clear(t);
    cdlist_append(&list_blockedRecv, &t->schedList);
}

void sched_state_setBlockedSend(OSThread* t){
    sched_state_clear(t);
    cdlist_append(&list_blockedSend, &t->schedList);
}

uint8_t sched_state_isBlockedRecv(OSThread* t){
    return cdlist_contains(&list_blockedRecv, &t->schedList);
}

/*uint8_t sched_state_isBlockedInterrupt(OSThread* t){
    return cdlist_contains(&list_blockedInterrupt, &t->schedList);
}*/



static void wakeSleepingThreads(void){
    Time now = os_time_now();

    // (Cannot use foreach, because we modify the list while iterating over it)
    for(CDListItem* item = list_sleeping.next;
        item != &list_sleeping;
    ){
        OSThread* t = enclosing(item, OSThread, schedList);
        if(os_time_cmp(now, t->wakeTime) >= 0){
            cdlist_unlink(item);
            item = item->next;
            sched_state_setRunnable(t);
        } else {
            item = item->next;
        }
    }
}

static void checkStackOverflow(OSThread* t){
    os_assert(t->stack.ptr >= t->stack.location, 
        "Overflowed stack by %d bytes",
            t->stack.location - t->stack.ptr
    );
}

static void reschedule(void){

    //PORTB ^= (1 << PB4);

    checkStackOverflow((OSThread*)currentThread);
    wakeSleepingThreads();
    
    /*
    uart_printfUnlocked("schedLists:\n");
    schedLists_foreach(list, {
        uart_printfUnlocked("  List: %p\n", list);
        cdlist_foreach(item, list){
            OSThread* t = enclosing(item, OSThread, schedList); 
            uart_printfUnlocked("    Thread(%p, %s)\n", (void*)t, t->name ?: "");
        }
    });
    uart_printfUnlocked("\n");
    */
    

    uint8_t numRunnable = cdlist_length(&list_runnable);

    if(numRunnable != 0){
        uint8_t r = rand_r(&threadSelectRandCtx) % numRunnable;
        currentThread = enclosing(cdlist_idx(&list_runnable, r), OSThread, schedList);
        //uart_printfUnlocked("Running %p (#%d)\n\n", currentThread, currentThread->id);
    } else {
        currentThread = idleThread;
    }

}



/* Rationale for naked/asm: 
This function can be called at any time. If there is any prologue/epilogue, 
the epilogue may be run on the wrong stack.
*/
void sched_yield(void) __attribute__((naked));
void sched_yield(void){
    SAVE_CONTEXT();
    MOVE_SP(ctxSwitchStack);
    
    reschedule();

    RESTORE_CONTEXT();
    asm volatile ( "ret" );
}


void sched_yieldFromTick(void) __attribute__((naked));
void sched_yieldFromTick(void){
    SAVE_CONTEXT();
    MOVE_SP(ctxSwitchStack);
    
    time_incrementTick();
    reschedule();

    RESTORE_CONTEXT();
    asm volatile ( "ret" );
}


ISR(TIMER1_COMPA_vect, ISR_NAKED){
    sched_yieldFromTick();
    asm volatile ( "reti" );
}

void sched_yieldFromNowhere(void) __attribute__((naked));
void sched_yieldFromNowhere(void){
    MOVE_SP(ctxSwitchStack);

    currentThread = idleThread;
    reschedule();

    RESTORE_CONTEXT();
    asm volatile ( "ret" );
}





OSThread* sched_currentThread(void){
    OSThread* t;
    atom t = (OSThread*)currentThread;
    return t;
}

uint8_t sched_isAlive(OSThread* t){
    atom threads_foreach(a, {
        if(t == a){
            return 1;
        }
    });
    return 0;
}

void sched_assignNullOwnerToSpawnOf(OSThread* terminatedThread){
    atom threads_foreach(t, {
        if(t->owner == terminatedThread){
            t->owner = NULL;
        }
    });
}



static void createCtxSwitchStack(void){
    ctxSwitchStack = os_malloc(ISR_STACK_SIZE);
    ctxSwitchStack += ISR_STACK_SIZE - 1;
}

static void makeEntryPointIntoThread(void){
    OSThread* t = osthread_new(0);

    t->owner            = NULL;
    t->name             = "os_main";

    currentThread = t;
    sched_state_setRunnable(t);
}

static void startSchedTimerInterrupt(void){

    TCCR1B  |=  (1 << WGM12);

    // Prescaler
    TCCR1B  |=  (1 << CS10) 
            |   (1 << CS11);

    // Output compare match every TICK'th second
    OCR1A = F_CPU / 64/*Prescaler*/ / TICK_HZ;

    // Enable interrupt for timer 1A
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

static void idleThreadFn(void){

    while(1){}

    /*
    while(1){
        atom {
            set_sleep_mode(SLEEP_MODE_PWR_SAVE);
            sleep_enable();
        }
        sleep_cpu();
        sleep_disable();
    }
    */
    
    /*
    // Possibly an alternative?
    atom {
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        sleep_enable();
    }
    while(1){
        sleep_cpu();
    }
    */
}

static void createIdleThread(void){
    idleThread = osthread_new(100);
    idleThread->stack.ptr = osthread_initializeStack(idleThread->stack.ptr, idleThreadFn);
}



void sched_init(void){
    cli();
    
    schedLists_foreach(list, {
        cdlist_reset(list);
    })
    
    threadSelectRandCtx = os_unpredictableSeed();
    createCtxSwitchStack();
    makeEntryPointIntoThread();
    createIdleThread();
    startSchedTimerInterrupt();
}






