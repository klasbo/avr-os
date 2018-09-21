#include "../os.h"

/*
This example illustrates basic message passing (os_send/os_receive/os_reply).

The "accumulator" thread behaves much like a stateful object: It adds all the
 values it receives, and sends back the result when asked.
Empty structs can be used as "signal" messages. Note that they must be given 
 an identifier in `os_receive`, even though the "variable" is just a dummy.
To reply to the sender of a particular message, use `os_reply`. In cases 
 where you want to reply to the owner of the thread (rather than the sender 
 of the message), use `os_send(os_ownerThread(), [...])`.
*/


typedef struct {} GetResult;
typedef struct {} Quit;

static void accumulator(void){

    int16_t sum = 0;

    while(1){
        os_receive(
            int8_t, v, {
                //uart_printf("Received: %d\n", v);
                sum += v;
            },
            GetResult, gr, {
                os_reply(int16_t, sum);
            },
            Quit, q, {
                os_terminateThread();
            }
        );
    }

}


void examples_messages(void){
    //uart_printf("\nRunning %s\n", __FUNCTION__);

    // Spawn the accumulator as a thread
    Thread accumulatorThread = os_spawn(accumulator, 60);
    
    // Send the accumulator some values
    for(int8_t i = 0; i < 10; i++){
        os_send(accumulatorThread, int8_t, i);
    }

    // Ask for the result, and receive it
    os_send(accumulatorThread, GetResult, {});
    os_receive(
        int16_t, sum, {
            //uart_printf("Total: %d\n", sum);
            os_assert(sum == 45, "%s failure", __FUNCTION__);
        }
    );

    // Clean up
    os_send(accumulatorThread, Quit, {});
    
    //uart_printf("%s complete\n\n", __FUNCTION__);
}
