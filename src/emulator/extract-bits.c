#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "constants.h"
#include "armv8.h"
#include "extract-bits.h"

//Extracts specific bits from a 32 bit integer
//Takes in the int to extract from, a start index, and number of bits to extract
unsigned int extract_bits(uint32_t value, int index, int num_bits) {
	//Work out the bit mask
	uint32_t bit_mask = (1 << num_bits) - 1;

	//Shift by index and AND with bit mask
	return ((value >> index) & bit_mask);	
}


//extract bits from memory
int get_memory_data(armv8_state *armv8, uint64_t addr, int num_bytes, uint64_t *data) {
	if (addr + WORD_SIZE_32 > MEM_SIZE) {
		fprintf(stderr, "Invalid memory address");
		return 1;
	}
	assert(addr + WORD_SIZE_32 <= MEM_SIZE);

	*data = 0;

	for (int i = 0; i < num_bytes; i++) {
		*data |= (uint64_t)(armv8->memory[addr + i]) << (WORD_SIZE_64 * i); 
	}

	return 0;
}


