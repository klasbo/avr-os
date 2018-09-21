
#include "../util/atom.h"

#include <avr/eeprom.h>
#include <stdlib.h>

EEMEM uint32_t seed;

__attribute__((constructor)) uint32_t os_unpredictableSeed(void){
    uint32_t i;
    eeprom_read_block(&i, &seed, sizeof(seed));
    i = rand_r(&i);
    eeprom_write_block(&i, &seed, sizeof(seed));
    return i;
}

int os_rand(void){
    int r;
    atom r = rand();
    return r;
}