#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "data-processing.h"
#include "modify-regs.h"
#include "sign-extension.h"
#include <limits.h>
#include <assert.h>


//Bitwise shift operations

//Shifts operand to the left, inserting zeros from least significant bit.
static uint64_t logical_shift_left(armv8_state *armv8, uint64_t operand, int shift, int width) {
	if (width == 32) {
		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
		return ((uint32_t)(operand) << shift);
    	}
    	return operand << shift;
}

//Shifts operand to the right, inserting zeros from most significant bit.
static uint64_t logical_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
    	if (width == 32) {
		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
        	return ((uint32_t)(operand) >> shift);
    	}
    	return operand >> shift;
}

//Operand shifted to the right and the most significant bit is copied into vacant positions.
static uint64_t arithmetic_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) { 
    	if (width == 32) { 
        	int32_t shifted32 = ((int32_t)(operand & 0xffffffff)) >> shift;
        	// Zero-extend back to 64 bits
		return (uint64_t)shifted32;
    	} else {
		//Casting to signed int32 to ensure sign-extension on >>
        	int64_t shifted64 = (int64_t)operand >> shift; 
		return (uint64_t)shifted64;
    	}
}

//Operand shifted to the right, wrapping the shifted bits around
static uint64_t rotate_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
	if (width == 32) {
		uint32_t version32 = (uint32_t)operand;
		return (version32 >> shift) | (version32 << (32 - shift));

	} else {
		return (operand >> shift) | (operand << (64 - shift));
	}
}


//Performs arithmetic
//Takes arguments: armv8 state pointer, opcode, 2 signed 64 bit int arguments, bit width
//Returns 64 bit signed int
static int64_t perform_arithmetic(armv8_state *armv8, int opcode, int64_t arg1, int64_t arg2, int width) {
	int64_t result = UINT64_MAX;

	//mask operands if 32 bits
	if (width == 32) {
		arg1 = (int32_t)arg1;
		arg2 = (int32_t)arg2;
	}

	switch(opcode){
		case 0:  //add
			result = arg1 + arg2;
			break;
		
		case 1:  //adds
			result = arg1 + arg2;
			update_pstate(&armv8->PSTATE, (uint64_t) arg1, (uint64_t) arg2, result, OP_ADD, 64);
			break;

		case 2:  //sub
			result = arg1 - arg2;
			break;

		case 3:  //subs
			result = arg1 - arg2;
			update_pstate(&armv8->PSTATE, (uint64_t) arg1, (uint64_t) arg2, result, OP_SUB, 64);
			break;

		default: 
			fprintf(stderr, "Error. Unknown arithmetic opcode.");
			return 1;
			break;
	}

	//mask result if 32 bit
	if (width == 32) {
		result = (int32_t)result;
	}

	return result;
}


//Immediate data processing instructions
//Takes arguments: 32 bit instruction, armv8 state pointer
//Returns 0 for success, 1 for failure
int immdp(uint32_t instr, armv8_state *armv8) {
	unsigned int width = ((instr >> 31) & 0x1) ? 64 : 32; //MSB = 1: Width = 64, MSB = 0: Width = 32
	unsigned int opi = (instr >> 23) & 0x7; //Data processing operation 010=Arithmetic, 101=Wide move
	unsigned int opc = (instr >> 29) & 0x3; //Operation code
	unsigned int rd = instr & 0x1f; //Destination register
	int64_t result;

	//variables for arithmetic instructions
	unsigned int shift = ((instr >> 22) & 0x1) ? 12 : 0; //shift by 12 if bit 22 = 1 
	uint64_t imm = ((instr >> 10) & 0xfff) << shift; //shifted immediate value 
	unsigned int rn = (instr >> 5) & 0x1f; //1st operand register
	uint64_t op = 0;

	//variables for wide move
	int hw = ((instr >> 21) & 0x3);

	switch(opi){
		case 2: //arithmetic			
			//performs arithmetic based on bit width
			if (read_reg(armv8, rn, &op, width)) { return 1; }
			result = perform_arithmetic(armv8, opc, op, imm, width);
			break;

		case 5: //wide move
			
			if (width == 32 && hw > 1) {
				fprintf(stderr, "Invalid shift for 32 bit");
				return 1;
			}
			int shift = 16 * hw;

			uint64_t imm = ((uint64_t)((instr >> 5) & 0xffff)) << shift; // shifted immediate value
			uint64_t value;

			switch(opc){
				case 0: //move wide with NOT
					result = ~(imm);
					break;
				
				case 2: //move wide with zero
					result = imm;
					break;

				case 3: //move wide with keep
					//reads rd register 	
                        		if (read_reg(armv8, rd, &value, 64)) { return 1; }

					//Masking bits
					result = (value) & ~((uint64_t)0xffff << shift); //set appropriate 16 bits to zero
					result = result | imm; //move imm into these 16 bits
					break;

				default:
					fprintf(stderr, "Unknown OPC for wide move instruction.");
					return 1;
			}
			break;

		default: 
			fprintf(stderr, "Unknown OPI in immediate data processing instruction.");
			return 1;
			break;
		
	}
	
	// If destination is 0 register, do not write. 
	if (rd == 0x1f) { return 0; }
	
	return write_reg(armv8, rd, result, width);
}

int regdp(uint32_t instr, armv8_state *armv8) {
	unsigned int opr = (instr >> 21) & 0xf; //opr = bits 21-24 of instruction
	unsigned int type = opr | (((instr >> 28) & 0x1) << 4); //M-opr: type of instruction (M = bit 28)
	unsigned int operand = (instr >> 10) & 0x3f; //last operand
	unsigned int rd = instr & 0x1f; //destination register
	unsigned int rn = (instr >> 5) & 0x1f; //register operand
	unsigned int rm = (instr >> 16) & 0x1f; //register that shift is performed on
	unsigned int opc = (instr >> 29) & 0x3; //opcode
	uint64_t op1 = 0; //first operand
	uint64_t op2 = 0; //second operand
	int width = ((instr >> 31) & 0x1) ? 64 : 32; //width depending on MSB
	int64_t result;
	
	//check that operand is in the valid range
	if (operand > 63 || (type != 24 && (width == 32 && operand > 31))) {
		fprintf(stderr, "Invalid operand");
		return 1;
	}

	//reads register rn into op1, if rn is not ZR
	if (rn != 0x1f) {
		if (read_reg(armv8, rn, &op1, width)) { return 1; }
	}

	//reads register rm into op2, if rm is not ZR
	if (rm != 0x1f) {
		if (read_reg(armv8, rm, &op2, width)) { return 1; }
	}

	//perform shift
	if(type != 24) {
		
		//Checks if the shift amount is within the valid range
		if (operand < 0 || operand > width - 1) {
			fprintf(stderr, "Invalid shift amount");
			return 1;
		}

		switch((opr >> 1) & 0x3){ //shift operand bits
			case 0: //lsl
				op2 = logical_shift_left(armv8, op2, operand, width); //defined in modify-regs.c 
				break;

			case 1: //lsr
				op2 = logical_shift_right(armv8, op2, operand, width);
				break;

			case 2: //asr
				op2 = arithmetic_shift_right(armv8, op2, operand, width);
				break;

			case 3: //ror
				if (type < 8) {
					op2 = rotate_right(armv8, op2, operand, width);
				} else {
					fprintf(stderr, "Unknown shift type for arithmetic instructions");
					return 1;
				}
				break;
			
			default: 
			        fprintf(stderr, "Unknown shift type");
				return 1;
		}
		
	}
	if(type < 8){
		//logical
		bool negate = opr & 0x1;
		
		if(negate) { op2 = ~op2; }

		switch(opc){ //specifies operation
			case 0: //and
				result = op1 & op2;
				break;

			case 1: //or
				result = op1 | op2;
				break;

			case 2: //xor
				result = op1 ^ op2;
				break;

			case 3: //and, set flags
				result = op1 & op2;
				update_pstate(&armv8->PSTATE, op1, op2, result, OP_LOGIC, width);
				break;

			default: 
				fprintf(stderr, "Invalid operation code for logical dp operation");
				return 1;
		}

	}else if(type < 16 && type % 2 == 0){ 
		//arithmetic
		result = perform_arithmetic(armv8, opc, op1, op2, width); 	

	}else if(type == 24){
		//multiply
		bool negate = (operand >> 5) & 0x1; //madd if 0/false, msub if 1/true
		unsigned int ra = operand & 0x1f;

		//read in ra register, if ra is not ZR
		uint64_t op3 = 0;
		if (ra != 0x1f) {
			if (read_reg(armv8, ra, &op3, width)) { return 1; }
		}
		
		//sign extend the operands to avoid signed arithmetic errors
		op1 = sign_ext_64(op1, width);
		op2 = sign_ext_64(op2, width);
		op3 = sign_ext_64(op3, width);

		if(negate){
			result = op3 - (op1 * op2);
		}else{
			result = op3 + (op1 * op2);
		}
	}else{
		fprintf(stderr, "Unknown type of data processing register instruction.");
		return 1;
	}
	
	// If zero register, do not write 
	if (rd == 0x1f) { return 0; }

	return write_reg(armv8, rd, result, width);

}



