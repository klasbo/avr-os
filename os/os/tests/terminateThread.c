
#include "../os.h"
#include "../core/internal/sched.h"


static Resource res;

static void spawnedFn(void){
    os_resource_acquire(&res);
    os_terminateThread();
}

void test_terminateThread_releasesResource(void){
    os_resource_init(&res);
    Thread t = os_spawn(spawnedFn, 60);
    while(sched_isAlive(t)){
        os_yield();
    }
    with(&res){}
}



