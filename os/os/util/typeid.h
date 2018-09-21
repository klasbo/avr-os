
#pragma once


// Generate a (probably) unique uint16_t number for a type
//  Uses the string representation of the type, so `T *` and `T*` 
//  end up being two different types
#define typeid(t) hash(str(t))


#include <string.h>

#define str(s) #s

#define hash(s)     ((uint16_t)(H64(s,0,0)))
#define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))


