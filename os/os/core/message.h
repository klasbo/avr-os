#pragma once

#include <stdint.h>

#include "../util/cpp_magic.h"
#include "../util/typeid.h"
#include "thread.h"


// Send a message to another thread.
//  Sending a message is a rendezvous: The sending thread will block until 
//   the message has been received.
//
// Arguments:
//  thread:      The receiver thread (must be of type `Thread`).
//  type:        The type of the message (See "Implementation notes" below).
//  initializer: The value to send.
//  varargs:     Just to allow commas in the initializer.
//
// Example:
//      Thread someThread;
//
//      os_send(someThread, uint8_t, 5);
//
//      typedef struct {
//          uint8_t i;
//          uint8_t j;
//      } SomeStruct;
//
//      os_send(someThread, SomeStruct, {.j = 4, .i = 3});
//
//      // Can also use an existing value as the initializer
//      SomeStruct ss = (SomeStruct){.i = 5, .j = 3};
//      os_send(someThread, SomeStruct, ss);
//
// Implementation notes:
//  - Types are matched on their string representation, so `T*` and `T *` 
//    are not the same type. The same applies to typedef/#define'd aliases.
//  - Since sending a message is a rendezvous, you cannot send a message to 
//    yourself (ie `os_send(os_thisThread(), [...])`).
//  - You cannot send a message while holding a resource.
//  - The implementation asserts that the intended receiver is a valid 
//    (alive) thread. 
#define os_send(thread, type, initializer, ...)                             \
    {                                                                       \
        type _v = (type) MAP(PASS, COMMA, initializer, __VA_ARGS__);        \
        _msg_send(thread, typeid(type), &_v);                               \
    }



// Receive a message from another thread.
//  Receiving a message is a rendezvous: The receiving thread will block 
//   until another thread sends it a message.
//  Messages are pattern-matched to types, and the corresponding statement 
//   is executed.
//  An assert is triggered if the type of received message does not match 
//   any of the given types.
//  `os_reply` is available from within the statement.
//
// Arguments:
//  type:       The type of the message (See "Implementation notes" below).
//  identifier: The name of the variable available in the statement.
//  statement:  Block of statements to run when the received message matches 
//              the type.
//  varargs:    Additional {type, identifier, statement} triplets.
//
// Example:
//      typedef struct {
//          uint8_t i;
//          uint8_t j;
//      } SomeStruct;
//
//      typedef struct {} RequestMsg;
//
//      uint8_t current = 0;
//      while(1){
//          os_receive(
//              uint8_t, i, {
//                  uart_printf("Received new value: %d\n", i);
//                  current = i;
//              },
//              RequestMsg, _, {
//                  uart_printf("Received request for value\n");
//                  os_reply(uint8_t, current);
//              },
//              SomeStruct, ss, {
//                  uart_printf("Received SomeStruct(i:%d, j:%d)\n", ss.i, ss.j);
//              }
//          );
//      }
//
// Implementation notes:
//  - Types are matched on their string representation, so `T*` and `T *` 
//    are not the same type. The same applies to typedef/#define'd aliases.
//  - Duplicated types (ie hash collisions) are not detected, because 
//    avr-gcc does not implemented "-Wduplicated-cond". (The pragma is 
//    currently commented out, and should be enabled as soon as the compiler
//    supports it).
#define os_receive(type, identifier, statement, ...)                        \
    {                                                                       \
        /*_Pragma("GCC diagnostic push")*/                                  \
        /*_Pragma("GCC diagnostic warning \"-Wduplicated-cond\"")*/         \
        _msg_recvWait();                                                    \
        uint16_t _type = _msg_type();                                       \
        void*    _addr = _msg_addr();                                       \
        Thread _sender __attribute__((unused)) = _msg_sender();             \
        if(0){                                                              \
        MAP_TRIPLETS(_message_recvCase, EMPTY, type, identifier, statement, __VA_ARGS__) \
        } else {                                                            \
            os_assert(0, "Received unexpected message type %d\n", _type);   \
        }                                                                   \
        /*_Pragma("GCC diagnostic pop")*/                                   \
    }



// Send a reply to the sender of the message currently being received.
//  `os_reply` is only valid as part of a `statement` passed to `os_receive`.
//  See `os_send` for arguments.
//  See `os_receive` for example.
#define os_reply(type, initializer, ...)                                    \
    /*how to provide informative errmsg if reply is used outside receive?*/ \
    /*equivalent of `static assert(__traits(compiles, _sender));` ?*/       \
    os_send(_sender, type, initializer, __VA_ARGS__)




    

// Receive a message from another thread, but ignore messages that do not match
//  See `os_receive`.
//  An assert is *not* triggered when the type of received messages fails to 
//   match. Instead, `os_receiveIgnoreUnexpected` will continue to receive
//   invalid messages until a valid match is found.
//  Unexpected messages are lost forever. You cannot ignore some types of 
//   messages "for now", and get back to them later.
//  The usefulness of this function is probably severely limited.
#define os_receiveIgnoreUnexpected(type, identifier, statement, ...)        \
    {                                                                       \
        uint8_t _noMatch = 0;                                               \
        do {                                                                \
            _noMatch = 0;                                                   \
            _msg_recvWait();                                                \
            uint16_t _type = _msg_type();                                   \
            void*    _addr = _msg_addr();                                   \
            Thread _sender __attribute__((unused)) = _msg_sender();         \
            if(0){                                                          \
            MAP_TRIPLETS(_message_recvCase, EMPTY, type, identifier, statement, __VA_ARGS__) \
            } else {                                                        \
                _noMatch = 1;                                               \
                _msg_recvDone();                                            \
            }                                                               \
        } while(_noMatch);                                                  \
    }







// (Internal)
#define _message_recvCase(type, identifier, statement)                      \
    } else if(_type == typeid(type)){                                       \
        type identifier;                                                    \
        memcpy(&identifier, _addr, sizeof(type));                           \
        _msg_recvDone();                                                    \
        statement


// (Internal)
// Do not call any of these functions "manually".
// Use `os_send`/`os_receive`/`os_reply`
void _msg_send(Thread receiver, uint16_t type, void* addr);
void _msg_recvWait(void);
void _msg_recvDone(void);
void* _msg_addr(void);
uint16_t _msg_type(void);
Thread _msg_sender(void);


