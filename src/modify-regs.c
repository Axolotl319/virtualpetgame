#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "modify-regs.h"
#include "armv8.h"

//checks if registers are in bounds - returns 1 if they aren't
int check_reg_bounds(int reg_num) {
	return (reg_num < 0 || reg_num >= NUM_GP_REGS);
}


//32 bit register operations - read and write
//Reads to a pointer - returns 1 if failure, 0 if success
int read_32(armv8_state *armv8, int reg_num, uint32_t *data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	
	//Sets upper 32 bits to 0
	*data = ((uint32_t)(armv8->GP_regs[reg_num] && 0xffffffff));
	return 0;
}

int write_32(armv8_state *armv8, int reg_num, uint32_t data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	armv8->GP_regs[reg_num] = ((uint64_t) data); 
	return 0;
}

//64 bit register operations - read and write
int read_64(armv8_state *armv8, int reg_num, uint64_t *data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	*data = armv8->GP_regs[reg_num];
	return 0;
}

int write_64(armv8_state *armv8, int reg_num, uint64_t data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	armv8->GP_regs[reg_num] = data;
	return 0;
}

//PC operations
//int incrementPC(armv8_state *armv8) {}
//int modifyPC(armv8_state *armv8) {}

//Pstate operations


