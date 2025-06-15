#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "constants.h"
#include "armv8.h"
#include <assert.h>

//checks if registers are in bounds - returns 1 if they aren't
static int check_reg_bounds(int reg_num) {
	return (reg_num < 0 || reg_num >= NUM_GP_REGS);
}

//Write to GP register
//Takes arguments: armv8 state, register number, 64 bit data to write, register width
int write_reg(armv8_state *armv8, int reg_num, uint64_t data, int width) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}

	if (width == WIDTH_32) {
		data &= MASK_32; //mask 32 bits only
	}

	armv8->GP_regs[reg_num] = data;
	return 0;
}

//Reads GP register
//Takes arguments: armv8 state pointer, register number, 64 bit int pointer for data, register width 
int read_reg(armv8_state *armv8, int reg_num, uint64_t *data, int width) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}

	uint64_t raw_data = armv8->GP_regs[reg_num];

	//Sets upper 32 bits to 0 if width is 32
	*data = (width == WIDTH_32) ? 
		(uint64_t)((uint32_t)raw_data) :
		(raw_data);

	return 0;
}


//PC operations
//Increment - takes argument armv8 state pointer
void incrementPC(armv8_state *armv8) {
	armv8->PC+=WORD_SIZE_32;
}

//Sets PC to a specified address
//Takes arguments armv8 state pointer and 64 bit int address
int setPC(armv8_state *armv8, uint64_t addr) {
	if (addr >= MEM_SIZE) { //PC too large
		fprintf(stderr, "Address out of bounds\n");
		return 1;
	}
       	assert(addr < MEM_SIZE);	
	armv8->PC = addr;
	return 0;
}


//Pstate operations

//updates pstate values depending on the operation and bit width
//Takes arguments: pstate pointer, 1st operand, 2nd operand, result, operation type, bit width
void update_pstate(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result, operation op_type, int width) {

	//Negative and Zero flags
	PSTATE->N = (result >> (width - 1)) & 1;
	PSTATE->Z = (result == 0);
	
	//sign bits for overflow checking
	int sign_op1 = (op1 >> (width - 1)) & 1;
	int sign_op2 = (op2 >> (width - 1)) & 1;
	int sign_result = (result >> (width - 1)) & 1;
	
	switch (op_type) {
		case OP_ADD: 
			//unsigned overflow
			//when the result is smaller than the operands
			PSTATE->C = (result < op1); 
		        
			//signed overflow
			//when the sign bit of the operands are same
			//and the sign bit of the result is different
			PSTATE->V = (sign_op1 == sign_op2) && (sign_result != sign_op1);
			break;
				

		case OP_SUB: 
			//Borrow = 0, No borrow = 1 
			PSTATE->C = (op1 >= op2);

			//signed overflow
			//when the sign bit of the operands are different
			//and the sign bit of the result is different to op1
			PSTATE->V = (sign_op1 != sign_op2) && (sign_result != sign_op1);
		        break;
		
		case OP_LOGIC: 
			PSTATE->C = 0;
			PSTATE->V = 0;
		        break;	

	}
}



//extract bits from memory
int get_memory_data(armv8_state *armv8, uint64_t addr, int num_bytes, uint64_t *data) {
	if (addr + WORD_SIZE_32 > MEM_SIZE) {
		fprintf(stderr, "Invalid memory address\n");
		return 1;
	}
	assert(addr + WORD_SIZE_32 <= MEM_SIZE);

	*data = 0;

	for (int i = 0; i < num_bytes; i++) {
		*data |= (uint64_t)(armv8->memory[addr + i]) << (WORD_SIZE_64 * i); 
	}

	return 0;
}


