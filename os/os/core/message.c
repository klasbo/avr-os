
#include "../util/assert.h"
#include "../util/atom.h"
#include "internal/osthread.h"
#include "internal/sched.h"
#include "resource.h"


void* _msg_addr(void){
    return sched_currentThread()->messageBox.addr;
}

uint16_t _msg_type(void){
    return sched_currentThread()->messageBox.typeId;
}

OSThread* _msg_sender(void){
    return enclosing(sched_currentThread()->messageBox.resource.handle.next, OSThread, blockList);
}


void _msg_send(OSThread* receiver, uint16_t typeId, void* addr){
    OSThread* thisThread = sched_currentThread();

    // This check is not a guarantee: It is possible that the receiver terminates just after this check
    os_assert(sched_isAlive(receiver), 
        "Tried to send a message to Thread(@%p, name:?), which does not exist\n",
        receiver);

    os_assert(receiver != thisThread, 
        "Tried to send a message to itself\n");

    // wait until we can grab the message box
    with(&receiver->messageBox.resource){
        atom {
            // copy message
            receiver->messageBox.typeId     = typeId;
            receiver->messageBox.addr       = addr;
            receiver->messageBox.unreadMsg  = 1;
    
            // make receiver runnable
            if(sched_state_isBlockedRecv(receiver)){
                // receiver arrived first & blocked; make it runnable again
                sched_state_setRunnable(receiver);
            } else {
                // receiver hasn't arrived yet; handled in recvWait
            }

            // wait for receiver to copy msg before we let someone else grab the message box
            sched_state_setBlockedSend(thisThread);
        }

        sched_yield();
        // receiver should now have copied the message
    }
}

void _msg_recvWait(void){
    OSThread* thisThread = sched_currentThread();

    // Cannot os_yield inside an atom: 
    //  check performed inside atom, os_yield performed outside.
    //  double yield (in case of "extra" yield from tick) has no ill effect.
    uint8_t waitingForMsg = 0;
    atom {
        if(thisThread->messageBox.unreadMsg == 0){
            // No sender yet; block
            waitingForMsg = 1;
            sched_state_setBlockedRecv(thisThread);       
        }
    }
    
    if(waitingForMsg){
        sched_yield();
        // Only way to become runnable is by being released by some sender
    }
}

void _msg_recvDone(void){
    atom {
        sched_currentThread()->messageBox.unreadMsg = 0;
        sched_state_setRunnable(_msg_sender());
    }
}



