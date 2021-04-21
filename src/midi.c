#include "midi.h"

uint16_t le2be16 (uint16_t number){
    return((number<<8)|    // move byte 0 to byte 1
           (number>>8));   // move byte 1 to byte 0
}

uint32_t le2be32 (uint32_t number){
    return(((number>>24)&0xff)       | // move byte 3 to byte 0
           ((number<<8) &0xff0000)   | // move byte 1 to byte 2
           ((number>>8) &0xff00)     | // move byte 2 to byte 1
           ((number<<24)&0xff000000)); // byte 0 to byte 3
} 