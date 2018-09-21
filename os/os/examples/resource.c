
#include "../os.h"

/*
This example shows how to use "Resources".

No two threads can acquire the `numberResource` at the same time, ensuring that
 the `sharedNumber` is only modified by one thread at a time.

Resources are not mutexes:
 - You can only acquire one resource at a time.
 - You can not hold a resource when sending a message.

Two methods of locking are demonstrated:
 - manual acquire/release
 - the `with`-statement
*/

Resource numberResource;
#define INITIAL_VALUE 1
int16_t sharedNumber = INITIAL_VALUE;

typedef struct {} WorkerDone;

static void adder(void){
    for(uint8_t i = 0; i < 254; i++){
        // Manual acquire/release: Familiar, but intertwines 
        //  "control-flow-code" with "logic-code"
        os_resource_acquire(&numberResource);
        sharedNumber++;
        os_resource_release(&numberResource);
    }

    //uart_printf("after add: %5d\n", sharedNumber);

    os_send(os_ownerThread(), WorkerDone, {});
    os_terminateThread();
}


static void subtracter(void){
    for(uint8_t i = 0; i < 254; i++){
        // The `with`-statement: Automatic acquire/release, making it
        //  impossible to forget to release the resource
        with(&numberResource){
            sharedNumber--;
        }
    }

    //uart_printf("after sub: %5d\n", sharedNumber);
    
    os_send(os_ownerThread(), WorkerDone, {});
    os_terminateThread();
}

void examples_resource(void){
    //uart_printf("\nRunning %s\n", __FUNCTION__);

    // Resource must be initialized before use
    os_resource_init(&numberResource);

    uint8_t numWorkers = 8;

    os_spawn(adder, 80);
    os_spawn(adder, 80);
    os_spawn(adder, 80);
    os_spawn(adder, 80);
    os_spawn(subtracter, 80);
    os_spawn(subtracter, 80);
    os_spawn(subtracter, 80);
    os_spawn(subtracter, 80);
    
    //uart_printf("Waiting for workers\n");
    uint8_t workersComplete = 0;
    while(1){
        os_receive(
            WorkerDone, wd, {
                workersComplete++;
                if(workersComplete == numWorkers){
                    break;
                }
            }
        );
    }

    //uart_printf("Final value: %d (should be %d)\n", sharedNumber, INITIAL_VALUE);
    os_assert(sharedNumber == INITIAL_VALUE, "%s failure", __FUNCTION__);
    
    //uart_printf("%s complete\n\n", __FUNCTION__);
}