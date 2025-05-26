#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "modify-regs.h"
#include "armv8.h"

//operation types for pstate
enum operation {
	OP_ADD,
	OP_SUB,
	OP_LOGIC
};


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
	*data = ((uint32_t)(armv8->GP_regs[reg_num] && 0xffffffff));
	return 0;
}

//Write
//Takes arguments armv8 state pointer, register number, data to write
//Returns 1 if failure, 0 if success
int write_reg32(armv8_state *armv8, int reg_num, uint32_t data) {
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

//PRIVATE update pstate function - functions to call defined below
//updates pstate values depending on the operation and bit width
static void update_pstate(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result, enum operation op_type, int width) {

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


//Call one of these to update pstate after an addition depending on bit width
//Takes arguments: pstate pointer, operand 1, operand 2, result
void update_pstate_add32(pstate *PSTATE, uint32_t op1, uint32_t op2, uint32_t result){
	update_pstate(PSTATE, (uint64_t) op1 , (uint64_t) op2 , (uint64_t) result, OP_ADD, 32);
}
void update_pstate_add64(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result){
	update_pstate(PSTATE, op1, op2, result, OP_ADD, 64);
}

//Call one of these to update pstate after a subtraction depending on bit width
//Takes arguments: pstate pointer, operand 1, operand 2, result
void update_pstate_sub32(pstate *PSTATE, uint32_t op1, uint32_t op2, uint32_t result){
 	update_pstate(PSTATE, (uint64_t) op1 , (uint64_t) op2 , (uint64_t) result, OP_SUB, 32);
}
void update_pstate_sub64(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result) {
        update_pstate(PSTATE, op1, op2, result, OP_SUB, 64);
}

//Call one of these to update pstate after a logical operation depending on bit width
//Takes arguments: pstate pointer, result
void update_pstate_logic32(pstate *PSTATE, uint32_t result){
	update_pstate(PSTATE, 0, 0, (uint64_t) result, OP_LOGIC, 32);
}
void update_pstate_logic64(pstate *PSTATE, uint64_t result){
	update_pstate(PSTATE, 0, 0, result, OP_LOGIC, 64);
}


