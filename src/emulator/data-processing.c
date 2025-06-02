#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "data-processing.h"
#include "modify-regs.h"
#include "sign-extension.h"
#include "extract-bits.h"
#include <limits.h>
#include <assert.h>

#define ZR 0x1f //Zero register

//struct for extracting bits from 32 bit instruction
typedef struct {
	int index;
	int bits;
} bit_range_t;

//immdp instruction format
typedef struct {
	bit_range_t rd;
	bit_range_t rn;
	bit_range_t imm12;
	bit_range_t sh;
	bit_range_t imm16;
	bit_range_t hw;
	bit_range_t opi;
	bit_range_t opc;
	bit_range_t sf;
} immdp_format_t;

const immdp_format_t immdp_format = {
	.rd    = {0, 5},
	.rn    = {5, 5},
	.imm12 = {10, 12},
	.sh    = {22, 1},
	.imm16 = {5, 16},
	.hw    = {21, 2},
	.opi   = {23, 3},
	.opc   = {29, 2},
	.sf    = {31, 1}
};

//regdp instruction format
typedef struct {
	bit_range_t rd;
	bit_range_t rn;
	bit_range_t operand;
	bit_range_t rm;
	bit_range_t opr;
	bit_range_t M;
	bit_range_t opc;
	bit_range_t sf;
} regdp_format_t;

const regdp_format_t regdp_format = {
	.rd      = {0, 5},
	.rn      = {5, 5},
	.operand = {10, 6},
	.rm      = {16, 5},
	.opr     = {21, 4},
	.M       = {28, 1},
	.opc     = {29, 2},
	.sf      = {31, 1}
};


//Bitwise shift operations

//Shifts operand to the left, inserting zeros from least significant bit.
static uint64_t logical_shift_left(armv8_state *armv8, uint64_t operand, int shift, int width) {
	if (width == WIDTH_32) {
		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
		return ((uint32_t)(operand) << shift);
    	}
    	return operand << shift;
}

//Shifts operand to the right, inserting zeros from most significant bit.
static uint64_t logical_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
    	if (width == WIDTH_32) {
		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
        	return ((uint32_t)(operand) >> shift);
    	}
    	return operand >> shift;
}

//Operand shifted to the right and the most significant bit is copied into vacant positions.
static uint64_t arithmetic_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) { 
	if (width == WIDTH_32) { 
		//truncate the top 32 bits before casting to signed int
        	int32_t shifted32 = ((int32_t)(uint32_t)(operand)) >> shift;
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
	if (width == WIDTH_32) {
		uint32_t version32 = (uint32_t)operand;
		return (version32 >> shift) | (version32 << (WIDTH_32 - shift));

	} else {
		return (operand >> shift) | (operand << (WIDTH_64 - shift));
	}
}


//Performs arithmetic
//Takes arguments: armv8 state pointer, opcode, 2 signed 64 bit int arguments, bit width
//Returns 64 bit signed int
static int64_t perform_arithmetic(armv8_state *armv8, int opcode, int64_t arg1, int64_t arg2, int width) {
	int64_t result = UINT64_MAX;

	//mask operands if 32 bits
	if (width == WIDTH_32) {
		arg1 = (int32_t)arg1;
		arg2 = (int32_t)arg2;
	}

	switch(opcode){
		case 0:  //add
			result = arg1 + arg2;
			break;
		
		case 1:  //adds
			result = arg1 + arg2;
			update_pstate(&armv8->PSTATE, (uint64_t) arg1, (uint64_t) arg2, result, OP_ADD, width);
			break;

		case 2:  //sub
			result = arg1 - arg2;
			break;

		case 3:  //subs
			result = arg1 - arg2;
			update_pstate(&armv8->PSTATE, (uint64_t) arg1, (uint64_t) arg2, result, OP_SUB, width);
			break;

		default: 
			fprintf(stderr, "Error. Unknown arithmetic opcode.");
			return 1;
			break;
	}

	//mask result if 32 bit
	if (width == WIDTH_32) {
		result = (int32_t)result;
	}

	return result;
}


//Immediate data processing instructions
//Takes arguments: 32 bit instruction, armv8 state pointer
//Returns 0 for success, 1 for failure
int immdp(uint32_t instr, armv8_state *armv8) {
	
	//Bit width
       	//MSB = 1: Width = 64, MSB = 0: Width = 32
	unsigned int width = extract_bits(instr, immdp_format.sf.index, immdp_format.sf.bits) ? WIDTH_64 : WIDTH_32;		
	//Data processing operation 010=Arithmetic, 101=Wide move
	unsigned int opi = extract_bits(instr, immdp_format.opi.index, immdp_format.opi.bits);
	//Operation code
	unsigned int opc = extract_bits(instr, immdp_format.opc.index, immdp_format.opc.bits);
	//Destination register
	unsigned int rd = extract_bits(instr, immdp_format.rd.index, immdp_format.rd.bits);
	//Result
	int64_t result;

	//Variables for arithmetic instructions
	int arith_shift_amount = 12; //shift is possibly 12
	unsigned int arith_shift = extract_bits(instr, immdp_format.sh.index, immdp_format.sh.bits) * arith_shift_amount; //Shift by 12 if bit 22 = 1
	//immediate 12 bit value
	uint64_t imm = extract_bits(instr, immdp_format.imm12.index, immdp_format.imm12.bits) << arith_shift; //shifted immediate value 
	//1st operand register
	unsigned int rn = extract_bits(instr, immdp_format.rn.index, immdp_format.rn.bits);
	uint64_t op = 0;

	//variables for wide move
	unsigned int hw = extract_bits(instr, immdp_format.hw.index, immdp_format.hw.bits);
	int mov_shift_amount = 16; //shift is hw*16
	unsigned int mov_shift = mov_shift_amount * hw;

	switch(opi){
		case 2: //arithmetic			
			//performs arithmetic based on bit width
			if (read_reg(armv8, rn, &op, width)) { return 1; }
			result = perform_arithmetic(armv8, opc, op, imm, width);
			break;

		case 5: //wide move
			
			if (width == WIDTH_32 && hw > 1) {
				fprintf(stderr, "Invalid shift for 32 bit");
				return 1;
			}
			
			//shifted immediate value
			uint64_t imm = ((uint64_t)
					(extract_bits(instr, immdp_format.imm16.index, immdp_format.imm16.bits))) 
					<< mov_shift;
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
                        		if (read_reg(armv8, rd, &value, WIDTH_64)) { return 1; }

					//Masking bits
					int bit_mask_16 = 0xffff;
					result = (value) & ~((uint64_t)(bit_mask_16) << mov_shift); //set appropriate 16 bits to zero
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
	if (rd == ZR) { return 0; }
	
	return write_reg(armv8, rd, result, width);
}

int regdp(uint32_t instr, armv8_state *armv8) {
	//regdp instruction format
	int opr_index = 21; //opr = bits 21-24
	int opr_bits = 4;
	int type_index = 28; //type flag = bit 28
	int type_bits = 1;
	int operand_index = 10; //last operand = bits 10-15
	int operand_bits = 6;
	int rd_index = 0; //rd = bits 0-4
	int rn_index = 5; //rn = bits 5-9
	int rm_index = 16; //rm = bits 16-20
	int reg_bits = 5;
	int opc_index = 29; //opc = bits 29-30
	int opc_bits = 2;
	int sf_index = 31; //sf = bit 31
	int sf_bits = 1;

	unsigned int opr = extract_bits(instr, opr_index, opr_bits); //opr = bits 21-24 of instruction
	unsigned int type = opr | (extract_bits(instr, type_index, type_bits) << 4); //M-opr: type of instruction (M = bit 28)
	unsigned int operand = extract_bits(instr, operand_index, operand_bits); //last operand
	unsigned int rd = extract_bits(instr, rd_index, reg_bits); //destination register
	unsigned int rn = extract_bits(instr, rn_index, reg_bits); //register operand
	unsigned int rm = extract_bits(instr, rm_index, reg_bits); //register that shift is performed on
	unsigned int opc = extract_bits(instr, opc_index, opc_bits); //opcode
	uint64_t op1 = 0; //first operand
	uint64_t op2 = 0; //second operand
	int width = (extract_bits(instr, sf_index, sf_bits)) ? WIDTH_64 : WIDTH_32; //width depending on MSB
	int64_t result;

	//instruction types
	int log_instr = 8; //logical instrs - type < 8
	int arith_instr = 16; //arith instrs - type < 16
	int mul_instr = 24; //mul instrs - type = 24


	
	//check that operand is in the valid range
	if (operand > 63 || (type != mul_instr && (width == WIDTH_32 && operand > 31))) {
		fprintf(stderr, "Invalid operand");
		return 1;
	}

	//reads register rn into op1, if rn is not ZR
	if (rn != ZR) {
		if (read_reg(armv8, rn, &op1, width)) { return 1; }
	}

	//reads register rm into op2, if rm is not ZR
	if (rm != ZR) {
		if (read_reg(armv8, rm, &op2, width)) { return 1; }
	}

	//perform shift
	if(type != mul_instr) {
		
		//Checks if the shift amount is within the valid range
		if (operand < 0 || operand > width - 1) {
			fprintf(stderr, "Invalid shift amount");
			return 1;
		}

		int shift_type_index = 1; //shift type = bits 1-2 of opr
		int shift_type_bits = 2;

		switch(extract_bits(opr, shift_type_index, shift_type_bits)){ //shift operand bits
			case 0: //lsl
				op2 = logical_shift_left(armv8, op2, operand, width); 
				break;

			case 1: //lsr
				op2 = logical_shift_right(armv8, op2, operand, width);
				break;

			case 2: //asr
				op2 = arithmetic_shift_right(armv8, op2, operand, width);
				break;

			case 3: //ror
				if (type < log_instr) {
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
	if(type < log_instr){
		//logical
		int negate_index = 0; //negate flag is bit 0 of opr
		int negate_bits = 1;
		bool negate = extract_bits(opr, negate_index, negate_bits);
		
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

	}else if(type < arith_instr && type % 2 == 0){ 
		//arithmetic
		result = perform_arithmetic(armv8, opc, op1, op2, width); 	

	}else if(type == mul_instr){
		//multiply
		int negate_index = 5; //negate flag is bit 5 of the operand
		int negate_bits = 1;
		int ra_index = 0; //ra is bits 0-4
		int ra_bits = 5;
		bool negate = extract_bits(operand, negate_index, negate_bits); //madd if 0/false, msub if 1/true
		unsigned int ra = extract_bits(operand, ra_index, ra_bits); 

		//read in ra register, if ra is not ZR
		uint64_t op3 = 0;
		if (ra != ZR) {
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
	if (rd == ZR) { return 0; }

	return write_reg(armv8, rd, result, width);

}



