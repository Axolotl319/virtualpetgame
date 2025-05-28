#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "modify-regs.h"
#include "armv8.h"


//checks if registers are in bounds - returns 1 if they aren't
static int check_reg_bounds(int reg_num) {
	return (reg_num < 0 || reg_num >= NUM_GP_REGS);
}


//32 bit register operations - read and write

//Read
//Takes arguments: armv8 state pointer, register number, 32 bit int pointer to store data
//Reads to the data pointer 
//Returns 1 if failure, 0 if success
int read_reg32(armv8_state *armv8, int reg_num, uint32_t *data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	
	//Sets upper 32 bits to 0
	*data = ((uint32_t)(armv8->GP_regs[reg_num] & 0xffffffff));
	return 0;
}

//Write
//Takes arguments armv8 state pointer, register number, 64 bit data to write
//Returns 1 if failure, 0 if success
int write_reg32(armv8_state *armv8, int reg_num, uint64_t data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	data &= 0xffffffff;
	armv8->GP_regs[reg_num] = data; 
	return 0;
}


//64 bit register operations - read and write

//Read
//Arguments same as 32 bit
int read_reg64(armv8_state *armv8, int reg_num, uint64_t *data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	*data = armv8->GP_regs[reg_num];
	return 0;
}

//Write 
//Arguments same as 32 bit
int write_reg64(armv8_state *armv8, int reg_num, uint64_t data) {
	if (check_reg_bounds(reg_num)) {
		fprintf(stderr, "Register out of bounds\n");
		return 1;
	}
	armv8->GP_regs[reg_num] = data;
	return 0;
}


//PC operations 
//Increment - takes argument armv8 state pointer
int incrementPC(armv8_state *armv8) {
	if (armv8->PC + WORD_SIZE >= MEM_SIZE) { 
		fprintf(stderr, "PC out of bounds\n");
		return 1;
	}
	armv8->PC+=WORD_SIZE;
	return 0;
}

//Sets PC to a specified address
//Takes arguments armv8 state pointer and 64 bit int address
int setPC(armv8_state *armv8, uint64_t addr) {
	if (addr >= MEM_SIZE) { //PC too large
		fprintf(stderr, "Address out of bounds\n");
		return 1;
	} 
	armv8->PC = addr;
	return 0;
}


//Pstate operations

//updates pstate values depending on the operation and bit width
//Takes arguments: pstate pointer, 1st operand, 2nd operand, result, operation type, bit width
void update_pstate(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result, enum operation op_type, int width) {

	//Negative and Zero flags
	PSTATE->N = (result >> (width - 1)) & 1;
	PSTATE->Z = (result == 0);
	
	//sign bits for overflow checking
	int sign_op1 = (op1 >> (width - 1)) & 1;
	int sign_op2 = (op2 >> (width - 1)) & 1;
	int sign_result = (result >> (width - 1)) & 1;
	
	switch (op_type) {
		case OP_ADD: {
			//unsigned overflow
			//when the result is smaller than the operands
			PSTATE->C = (result < op1); 
		        
			//signed overflow
			//when the sign bit of the operands are same
			//and the sign bit of the result is different
			PSTATE->V = (sign_op1 == sign_op2) && (sign_result != sign_op1);
			break;
		}		

		case OP_SUB: {	
			//Borrow = 0, No borrow = 1 
			PSTATE->C = (op1 >= op2);

			//signed overflow
			//when the sign bit of the operands are different
			//and the sign bit of the result is different to op1
			PSTATE->V = (sign_op1 != sign_op2) && (sign_result != sign_op1);
		        break;
		}
		case OP_LOGIC: {
			PSTATE->C = 0;
			PSTATE->V = 0;
		        break;
		}		

	}
}


//Bitwise shift operations

//Shifts operand to the left, inserting zeros from least significant bit.
uint64_t logical_shift_left(armv8_state *armv8, uint64_t operand, int shift, int width) {
    if (width == 32) {

		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
		return ((uint32_t)(operand) << shift);
    }
    return operand << shift;
}

//Shifts operand to the right, inserting zeros from most significant bit.
uint64_t logical_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
    if (width == 32) {

		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
        return ((uint32_t)(operand) >> shift);
    }
    return operand >> shift;
}

//Operand shifted to the right and the most significant bit is copied into vacant positions.
uint64_t arithmetic_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
    if (width == 32) {
        uint32_t version32 = (uint32_t)operand;

        //Mod shift by 32 to ensure shift value is within the valid range 
        int32_t shifted32 = (int32_t)version32 >> (shift % 32);

        // Zero-extend back to 64 bits
        return (uint32_t)shifted32;
    } else {
        //Mod shift by 64 to ensure shift value is within the valid range
		//Casting to signed int32 to ensure sign-extension on >>
        int64_t shifted64 = (int64_t)operand >> (shift % 64);
        return (uint64_t)(shifted64);
    }
}

//Performs a bitwise right rotate on the operand, preserving all bits.
uint64_t rotate_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
	if (width == 32) {

		//Mod shift by 32 to ensure shift value is within the valid range 
		shift %= 32;
		uint32_t version32 = (uint32_t)operand;
		uint32_t rotated_bits = (version32 >> shift) | (version32 << (32 - shift));
        return rotated_bits;
	} else {

		//Mod shift by 64 to ensure shift value is within the valid range
		shift %= 64;
		uint64_t rotated_bits = (operand >> shift) | (operand << (64 - shift));
        return rotated_bits;
	}
}