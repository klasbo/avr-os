
#include "../os.h"
#include "../core/internal/sched.h"

typedef struct {} Empty;

static void spawnedFn(void){
    Thread testThread = NULL;
    os_receive(Thread, t, { testThread = t; });
    while(os_ownerThread()){
        os_yield();
    }
    os_send(testThread, Empty, {});
}

static void ownerFn(void){
    Thread spawnedThread1 = os_spawn(spawnedFn, 60);
    Thread spawnedThread2 = os_spawn(spawnedFn, 60);
    os_receive(Thread, t, {
        os_send(spawnedThread1, Thread, t);
        os_send(spawnedThread2, Thread, t);
    });
    os_terminateThread();
}

void test_ownerThread_becomesNullOnOwnerTerminate(void){
    Thread ownerThread = os_spawn(ownerFn, 60);
    os_send(ownerThread, Thread, os_thisThread());
    os_receive(Empty, e, {});
    os_receive(Empty, e, {});
}