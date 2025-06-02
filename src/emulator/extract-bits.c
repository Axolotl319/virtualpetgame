#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "extract-bits.h"

//Extracts specific bits from a 32 bit integer
//Takes in the int to extract from, a start index, and number of bits to extract
unsigned int extract_bits(uint32_t value, int index, int num_bits) {
	//Work out the bit mask
	uint32_t bit_mask = (1 << num_bits) - 1;

	//Shift by index and AND with bit mask
	return ((value >> index) & bit_mask);	
}
