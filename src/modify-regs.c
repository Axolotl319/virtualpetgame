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

//Read
//Takes arguments: armv8 state, register number, 32 bit int pointer to store data
//Reads to the pointer 
//Returns 1 if failure, 0 if success
int read_32(armv8_state *armv8, int reg_num, uint32_t *data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	
	//Sets upper 32 bits to 0
	*data = ((uint32_t)(armv8->GP_regs[reg_num] && 0xffffffff));
	return 0;
}

//Write
//Takes arguments armv8 state, register number, data to write
//Returns 1 if failure, 0 if success
int write_32(armv8_state *armv8, int reg_num, uint32_t data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	armv8->GP_regs[reg_num] = ((uint64_t) data); 
	return 0;
}

//64 bit register operations - read and write

//Read
//Arguments same as 32 bit
int read_64(armv8_state *armv8, int reg_num, uint64_t *data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	*data = armv8->GP_regs[reg_num];
	return 0;
}

//Write 
//Arguments same as 32 bit
int write_64(armv8_state *armv8, int reg_num, uint64_t data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	armv8->GP_regs[reg_num] = data;
	return 0;
}

//PC operations - increment and set
int incrementPC(armv8_state *armv8) {
	if (armv8->PC + WORD_SIZE >= MEM_SIZE) { 
		fprintf(stderr, "PC out of bounds\n");
		return 1;
	}
	armv8->PC+=WORD_SIZE;
	return 0;
}

int setPC(armv8_state *armv8, uint64_t addr) {
	if (addr >= MEM_SIZE) { //PC too large
		fprintf(stderr, "Address out of bounds\n");
		return 1;
	} else if (addr % 4 != 0) { //PC not multiple of 4
		fprintf(stderr, "Invalid address\n");
		return 1;
	}

	armv8->PC = addr;
	return 0;
}

//Pstate operations


