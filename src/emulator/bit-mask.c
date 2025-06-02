#include <stdint.h>
#include <stdio.h>
#include <assert.h>

//Extracts specific bits from a 32 bit integer
//Takes in the 32 bit integer, a start index, and number of bits to extract
int extract_bits_32(uint32_t value, int index, int num_bits, unsigned int *extracted_val) {
	//Check if bits to extract are valid
	if (index < 0 || index + num_bits > 32) {
		fprintf(stderr, "Invalid index for bit extraction\n");
		return 1;
	}
	assert(index + num_bits <= 32);

	//Work out the bit mask
	uint32_t bit_mask = (1 << num_bits) - 1;
	printf("bitmask: %x\n", bit_mask);

	//Shift by index and AND with bit mask
	*extracted_val = (value >> index)  & bit_mask;
	return 0;	
}
