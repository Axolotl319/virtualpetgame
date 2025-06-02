#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "extract-bits.h"

//Extracts specific bits from a 32 bit integer
//Takes in the int to extract from, a start index, and number of bits to extract
unsigned int extract_bits(unsigned int value, int index, int num_bits) {
	//Check if bits to extract are valid
	/*if (index < 0 || index + num_bits > 32) {
		fprintf(stderr, "Invalid index for bit extraction\n");
		return 1;
	}
	assert(index + num_bits <= 32);*/

	//Work out the bit mask
	unsigned int bit_mask = (1 << num_bits) - 1;
	printf("bitmask: %x\n", bit_mask);

	//Shift by index and AND with bit mask
	return ((value >> index) & bit_mask);	
}
