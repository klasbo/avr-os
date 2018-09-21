

/*

Need some custom way to handle interrupts, because we cannot just use space from whatever stack we're currently on
    (we may not have the stack space, or all threads need extra overhead for isr's)

We want "context" for interrupts, or at least some way to communicate from ISR's to threads
    We cannot send messages, because 
        - interrupts cannot change sched state to "block on os_send" (msgpassing is rendezvous/synchronous)
        - interrupts have no persistent stack (msgpassing is zero-alloc)
    We can make interrupts run on the stack of a specific thread, by restoring that thread's context on reti
    We can shove messages in a malloc'd/free'd queue
        make popFront blocking (with semaphores) or just poll ( while(length==0){sched_yield();} )


Running interrupts on the stack of a specific thread
----------------------------------------------------
This assumes that the thread "expects" to have an interrupt run on it
If this is the case, we can do the following:
    ```
    ISR(vector, thread){
        save context
        move stack pointer to ISR stack
        set thread runnable
        set thread as currentThread in scheduler
        restore context
    }
    onInterrupt = 
        set thisThread state to blockedInterrupt
        yield
        // thisThread should now be running, it was awoken by someone
        disable interrupts (no yield must be possible between end of ISR and this statement)
        { statement list (the "ISR") }
        enable interrupts
    ```
This way, the interrupt runs on some thread's stack, so it can access variables/context for that thread
Problems: 
    1) this will seriously fuck up the thread's stack if it isn't expecting the interrupt
    2) we still cannot send messages from inside the statement list (just like we couldn't from the ISR), as we have disabled interrupts
        which means the ISR can trigger again whenever you *can* send a message (after interrupts are re-enabled)
    Both boil down to dealing with interrupts that are "unexpected" at some time
        (or sending messages when the OS isn't "running", but this is fundamentally unsolvable)
Solutions:
    a) ignore "unexpected" interrupts: I can't imagine this is smart, as the interrupt probably happened for a reason
        We can discard this "solution"
    b) crash the os: This is probably fine: we can reset the interrupt source after we have sent messages/whatever to the rest of the system
    c) increment some counter of "pending" interrupts
        This counter can overflow, in which case we are back to either a) or b)


Shoving messages in a queue
---------------------------
This queue can overflow (aka use all memory)
    And we're back to ignore/crash, but this time with some extra memory usage that can crash *other* parts of the system


The sensible solution
---------------------

The only sensible solution seems to be:
    - ISR wakes the waiting thread
    - OSThread runs "ISR" on its own stack, then re-enables interrupts
    - OSThread communicates with rest of system
    - OSThread resets interrupt source

For situations where we don't need to communicate with the rest of the system, we can just shove the "onInterrupt" handler in a 
while-loop (so that it always "expects" an interrupt). And if we don't need the context of a thread, we can still just use "bare" 
interrupts, as long as we move the stack pointer to the isr stack. Otherwise, there are some situations where neither will work:
    - all interrupt sources that are not manually reset
        - interrupt is a timer, so it can happen again before we loop back
            Use the OS timer. But this is a problem where WCET is the only "solution" anyway (ie unsolvable)
        - interrupt is on an external timer
            wtf? how does that even happen? Still unsolvable (WCET)
        - interrupt can happen at any time
            It's like a timer, but with random intervals. See above.
    - interrupt sources that need to be reset within some time
        This is a RT-problem. Fuckit.

These seem to only be timing problems, so this looks like a fine solution to interrupts for a strictly non-realtime OS...


Potential problems
------------------

Multiple interrupt vectors can trigger one thread's onInterrupt handler
    Can be solved by having the wake-thread-on-interrupt-macro also create a 
      `static uint8_t onlyOneInterruptPerTranslationUnit;` "variable"
      (or with the zero-size array static assert trick?)

    Though it could be useful!
        Multiple ISRs can run with the same context, so we can implement state machines inside the "ISR"
        Think of the possibilities! (and bugs)

    "onlyOneInterruptPerTU" should be default, but (maybe?) disable-able

*/


#if(0)

#define wakeThreadOn(vector, thread)                            \
    static Thread* thread = NULL;                               \
    ISR(vector, ISR_NAKED){                                     \
        SAVE_CONTEXT();                                         \
        MOVE_SP(isrStack);                                      \
        if(thread != NULL){    /* or assert? */                 \
            os_assert(sched_state_isBlockedInterrupt(thread),   \
                "OSThread was not expecting an interrupt");       \
            sched_state_setRunnable(thread);                    \
            sched_setCurrentThread(thread);                     \
        }                                                       \
        RESTORE_CONTEXT();                                      \
    }


#define onInterrupt(statement)                                  \
    cli();                                                      \    
        sched_state_setBlockedInterrupt(os_thisThread());       \
    sei();                                                      \
    sched_yield();                                              \
    cli();                                                      \
        statement                                               \
    sei();


/*
Usage example:
*/
wakeThreadOn(INT0_vect, canReceiverThread)
void canReceiver(void){
    canReceiverThread = os_thisThread();

    CanMessage msg = {0};

    can_init(); // enables can controller interrupts, must be done as "close as possible" to waitForInterrupt
    while(1){
        onInterrupt({
            // Nothing that needs to be done "immediately"
        })

        can_readMsg(&msg);

        switch(msg.id){
            case 1: os_send(someThread,      CanMessage, msg); break;
            case 2: os_send(someOtherThread, CanMessage, msg); break;
            default: break;
        }

        can_clearReadBuffer();
    }
}


#endif