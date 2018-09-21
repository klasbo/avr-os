#pragma once

// Gets a pointer to instance of type given ptr to memberName
// Arguments:
//  type:       some struct
//  ptr:        pointer to some member in that struct
//  memberName: name of that member
#define enclosing(ptr, type, memberName) \
    ((type*)( \
        (uint8_t*)(ptr) - (uint8_t*)(&member(type, memberName)) \
    ))


#define member(structType, memberName) \
    (((structType*)0)->memberName)

