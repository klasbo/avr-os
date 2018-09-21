

 - Multithreading
   - **Preemptive scheduling**: The scheduler is fully preemptive, and will run a *random* runnable thread each tick. The tickrate is configurable.
   - **Stack overflow detection**: New threads are given a fixed stack size when they are created. If a thread uses more stack space than it is given at the moment it is preempted, OSNAME will complain loudly and crash.

 - Message passing
   OSNAME aims to be thoroughly different, so it has message passing at its core.
   - **Synchronous (rendezvous)**: A sending thread will block until the receiving thread is ready.
   - **Asymmetric (with replies)**: Only the sender specifies which thread it is communicating with. The receiver does not specify who it is receiving a message from, but can reply specifically to the sender of the message it just received.
   - **Automatic pattern matching**: When a message is received it is automatically pattern-matched by type, and the associated block of statements is executed.
   - **Zero allocation**: Messages are copied from the stack of the sending thread to the stack of the receiving thread. There are no heap-allocated queues of messages that can grow uncontrollably.
 
 - Shared resource control
   A `Resource` is like a mutex that doesn't let you be creative. It allows for basic shared-variable control, but no more than absolutely necessary. 
   - **Thread-safe peripherals**: Hardware peripherals (such as UART) must be shared, so thread-safe abstractions are provided.
   - **No deadlocks**: You can only acquire one resource at a time. Deadlocks *shouldn't* be possible.
   - **No starvation**: Threads waiting for a resource are given access on a first-in first-out basis. No thread can be a resource hog.

 - Non-real-time
   OSNAME has no intention of being a real-time OS, so there are no priorities or protocols.
   - **Time & sleep**: You can get the time since the device was last reset, sleep for a fixed duration, or sleep until a specific time in the future. Threads become immediately runnable when they wake up, but the scheduler will only run the thread as soon as it is randomly selected.
   - **No priorities**: No problems with priority inversion or priority ceiling protocols.
   