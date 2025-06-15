#include <stdint.h>
#include "emulator-utils.h"
#include "constants.h"

//takes a number and msb to sign extend
int32_t sign_ext_32(int num, int msb_num) {
        int32_t shift = WIDTH_32 - msb_num;
	return (num << shift) >> shift;      
}

//takes an unsigned 64 bit integer and bit width
//returns 64 bit sign extended value
int64_t sign_ext_64(uint64_t value, int width) {
	return (width == WIDTH_32) ? 
		(int64_t)((int32_t)(value & MASK_32)) :
		(int64_t)value;
}

//Extracts specific bits from a 32 bit integer
//Takes in the int to extract from, a start index, and number of bits to extract
unsigned int extract_bits(uint32_t value, int index, int num_bits) {
	//Work out the bit mask
	uint32_t bit_mask = (1 << num_bits) - 1;

	//Shift by index and AND with bit mask
	return ((value >> index) & bit_mask);	
}

