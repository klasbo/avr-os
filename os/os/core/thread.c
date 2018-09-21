
#include "thread.h"

#include "../os_config.h"

#include "../util/atom.h"
#include "internal/sched.h"


void os_sleepUntil(Time time){
    atom {
        os_thisThread()->wakeTime = time;
        sched_state_setSleeping(os_thisThread());
    }
    sched_yield();
}

void os_sleep(Time dur){
    os_sleepUntil(os_time_add(os_time_now(), dur));
}

Thread os_thisThread(void){
    return sched_currentThread();
}

Thread os_ownerThread(void){
    Thread t;
    atom t = sched_currentThread()->owner;
    return t;
}

char* os_threadName(Thread t){
    char* n;
    atom n = t->name;
    return n;
}



static const uint16_t contextSize = 40;
Thread os_spawn(void fn(void), uint16_t stackSize){

    // Alloc OSThread, initialize fields
    Thread t = osthread_new(stackSize + contextSize);

    t->owner = os_thisThread();

    // Put stuff on the stack, so we can pop it the first time the scheduler 
    //  context-switches to the new thread
    t->stack.ptr = osthread_initializeStack(t->stack.ptr, fn);

    atom sched_state_setRunnable(t);

    return t;
}


Thread os_spawnNamed(void fn(void), uint16_t stackSize, char* name){
    // Spawned thread may assume that it has a name assigned, so we need to 
    //  make sure no context switch occurs before we have assigned the name
    Thread t = NULL;
    atom {
        t = os_spawn(fn, stackSize);
        t->name = name;
    }
    return t;
}


void os_terminateThread(void) __attribute__((naked));
void os_terminateThread(void){
    // After deleting this thread, we must reschedule & restore context
    // However, there is no context to save (as this thread is dead), 
    //  so we cannot call os_yield
    cli();

    Thread t = os_thisThread();

    sched_state_clear(t);
    sched_assignNullOwnerToSpawnOf(t);

    // We cannot be waiting for a resource (because then we would be waiting, not running), 
    //  so either we do not have a resource, or we are currently using a resource.
    if(!cdlist_empty(&t->blockList)){
        Resource* r = enclosing((&t->blockList)->prev, Resource, handle);
        os_resource_release(r);
    }
        
    // TODO: move stack before calling thread_delete
    osthread_delete(t);
        
    // Manually do only the "2nd half" of a yield
    sched_yieldFromNowhere();
    asm volatile ( "ret" );
}


void os_yield(void){
    sched_yield();
}