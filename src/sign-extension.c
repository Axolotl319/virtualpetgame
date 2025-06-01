#include <stdint.h>
#include "sign-extension.h"

//takes a number and msb to sign extend
int32_t sign_ext_32(int num, int msb_num) {
        int32_t shift = 32 - msb_num;
	return (num << shift) >> shift;      
}

//takes an unsigned 64 bit integer and bit width
//returns 64 bit sign extended value
int64_t sign_ext_64(uint64_t value, int width) {
	return (width == 32) ? 
		(int64_t)((int32_t)(value & 0xffffffff)) :
		(int64_t)value;
}

