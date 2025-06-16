#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "constants.h"
#include "armv8.h"
#include "emulator-utils.h"
#include "instr-formats.h"
#include "data-processing.h"
#include <limits.h>
#include <assert.h>

//immdp instructions
#define OPI_ARITH 0x2 //opi = 0b010 then immdp arithmetic instr
#define OPI_MOV   0x5 //opi = 0b101 then immdp wide move
//regdp instructions
#define ARITH_MASK 0x19 //arith instrs = 01xx0
#define ARITH_CODE 0x8
#define LOG_MASK 0x18 //log instrs = 00xxx
#define LOG_CODE 0x0
#define MUL_CODE 0x18 //mul instrs = 11000
//possible shifts for arith or move instructions
#define ARITH_SHIFT_AMT 12
#define MOV_SHIFT_AMT 16


//Get regdp instruction type
static regdp_instr_t get_instr_type(unsigned int mopr) {
	if ((mopr & ARITH_MASK) == ARITH_CODE) { return ARITH_INSTR; }
	if ((mopr & LOG_MASK) == LOG_CODE) { return LOG_INSTR; }
	if ((mopr == MUL_CODE)) { return MUL_INSTR; }
	return REGDP_INVALID;
}

//Bitwise shift operations

//Shifts operand to the left, inserting zeros from least significant bit.
static uint64_t logical_shift_left(armv8_state *armv8, 
		                   uint64_t operand, 
				   int shift, 
				   int width) {
	if (width == WIDTH_32) {
		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t 
		//to match function signature, so no cast needed.
		return ((uint32_t)(operand) << shift);
    	}
    	return operand << shift;
}

//Shifts operand to the right, inserting zeros from most significant bit.
static uint64_t logical_shift_right(armv8_state *armv8, 
				    uint64_t operand, 
				    int shift, 
				    int width) {
    	if (width == WIDTH_32) {
		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t 
		//to match function signature, so no cast needed.
        	return ((uint32_t)(operand) >> shift);
    	}
    	return operand >> shift;
}

//Operand shifted to the right 
//the most significant bit is copied into vacant positions.
static uint64_t arithmetic_shift_right(armv8_state *armv8, 
				       uint64_t operand, 
				       int shift, 
				       int width) { 
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
static uint64_t rotate_right(armv8_state *armv8, 
			     uint64_t operand, 
			     int shift, 
			     int width) {
	if (width == WIDTH_32) {
		uint32_t version32 = (uint32_t)operand;
		return (version32 >> shift) | (version32 << (WIDTH_32 - shift));

	} else {
		return (operand >> shift) | (operand << (WIDTH_64 - shift));
	}
}


//Performs arithmetic
//Takes arguments: armv8 state pointer, opcode, 
//	           2 signed 64 bit int arguments, bit width
//Returns 64 bit signed int
static int64_t perform_arithmetic(armv8_state *armv8, 
				  int opcode, 
				  int64_t arg1, 
				  int64_t arg2, 
				  int width) {

	int64_t result = UINT64_MAX;

	//mask operands if 32 bits
	if (width == WIDTH_32) {
		arg1 = (int32_t)arg1;
		arg2 = (int32_t)arg2;
	}

	switch(opcode){
		case ARITH_ADD:  //add
			result = arg1 + arg2;
			break;
		
		case ARITH_ADDS: //adds
			result = arg1 + arg2;
			update_pstate(&armv8->PSTATE, 
				     (uint64_t) arg1, 
				     (uint64_t) arg2, 
				     result, 
				     OP_ADD, 
				     width);
			break;

		case ARITH_SUB:  //sub
			result = arg1 - arg2;
			break;

		case ARITH_SUBS: //subs
			result = arg1 - arg2;
			update_pstate(&armv8->PSTATE, 
				     (uint64_t) arg1, 
				     (uint64_t) arg2, 
				     result, 
				     OP_SUB, 
				     width);
			break;

		default: 
			fprintf(stderr, "Error. Unknown arithmetic opcode.\n");
			return EXIT_FAILURE;
			break;
	}

	//mask result if 32 bit
	if (width == WIDTH_32) {
		result = (int32_t)result;
	}

	return result;
}

//helper function to carry out functionality of wide move based on move type
static int wide_move(int64_t *result, 
		     unsigned int opc,
		     uint64_t imm,
		     armv8_state *armv8,
		     unsigned int rd,
		     unsigned int mov_shift) {
	
	uint64_t value;
	
	switch(opc){
		case MOVN: //move wide with NOT
			*result = ~(imm);
			break;
				
		case MOVZ: //move wide with zero
			*result = imm;
			break;

		case MOVK: //move wide with keep
			//reads rd register 	
                     	if (read_reg(armv8, rd, &value, WIDTH_64)) {
				return EXIT_FAILURE; 
			}

			//Masking bits
			//set appropriate 16 bits to zero
			*result = (value) 
				  & ~((uint64_t)(MASK_16) 
				   << mov_shift); 

			//move imm into these 16 bits
			*result = *result | imm;
			break;

		default:
			fprintf(stderr, 
				"Unknown OPC for wide move instruction.\n");

			return EXIT_FAILURE;
			
			break;
		
	}
	return EXIT_SUCCESS;
}

//Immediate data processing instructions
//Takes arguments: 32 bit instruction, armv8 state pointer
//Returns 0 for success, 1 for failure
int immdp(uint32_t instr, armv8_state *armv8) {
	//Extract bits from instruction	
	//Bit width
       	//MSB = 1: Width = 64, MSB = 0: Width = 32
	unsigned int width = extract_bits(instr, 
			                  immdp_format.sf.index, 
					  immdp_format.sf.bits) 
		             ? WIDTH_64 : WIDTH_32;

	//Data processing operation 010=Arithmetic, 101=Wide move
	unsigned int opi = extract_bits(instr, 
			                immdp_format.opi.index, 
					immdp_format.opi.bits);

	//Opcode
	unsigned int opc = extract_bits(instr, 
			                immdp_format.opc.index, 
					immdp_format.opc.bits); 
	//Destination Register
	unsigned int rd = extract_bits(instr, 
			               immdp_format.rd.index, 
				       immdp_format.rd.bits); 
	//Result
	int64_t result; 

	//Variables for arithmetic instructions
	//Shift by 12 if bit 22 = 1
	unsigned int arith_shift = extract_bits(instr, 
						immdp_format.sh.index, 
						immdp_format.sh.bits) 
				   * ARITH_SHIFT_AMT; 
	//shifted immediate value	
	uint64_t imm = extract_bits(instr, 
				    immdp_format.imm12.index, 
				    immdp_format.imm12.bits) 
		       << arith_shift;

	//1st operand register
	unsigned int rn = extract_bits(instr, 
				       immdp_format.rn.index, 
				       immdp_format.rn.bits); 
	uint64_t op = 0;

	//variables for wide move
	//hw
	unsigned int hw = extract_bits(instr, 
				       immdp_format.hw.index, 
				       immdp_format.hw.bits); 
	//min val of hw for 32 bits
	int hw_min_32 = 1;
	//amount to shift by
	unsigned int mov_shift = MOV_SHIFT_AMT * hw;


	switch(opi){
		case OPI_ARITH: //arithmetic			
			//performs arithmetic based on bit width
			if (read_reg(armv8, rn, &op, width)) { 
				return EXIT_FAILURE; 
			}
			result = perform_arithmetic(armv8, opc, op, imm, width);
			break;

		case OPI_MOV: //wide move
			
			if (width == WIDTH_32 && hw > hw_min_32) {
				fprintf(stderr, "Invalid shift for 32 bit\n");
				return EXIT_FAILURE;
			}
			assert(width == WIDTH_64 || 
			      (width == WIDTH_32 && hw <= hw_min_32));
			
			//shifted immediate value
			uint64_t imm = ((uint64_t)
					(extract_bits(instr, 
						      immdp_format.imm16.index,
						      immdp_format.imm16.bits))) 
					<< mov_shift;

			if (wide_move(&result, opc, imm, armv8, rd, mov_shift)){
				return EXIT_FAILURE;
			}

			break;

		default: 
			fprintf(stderr, 
				"Unknown OPI in" 
				"immediate data processing instruction.\n");
			
			return EXIT_FAILURE;
			break;
		
	}
	
	// If destination is 0 register, do not write. 
	if (rd == ZRSP) { return EXIT_SUCCESS; }
	
	return write_reg(armv8, rd, result, width);
}


int regdp(uint32_t instr, armv8_state *armv8) {
	//opr = bits 21-24 of instruction
	unsigned int opr = extract_bits(instr, 
					regdp_format.opr.index, 
					regdp_format.opr.bits); 
	//M = bit 28
	unsigned int M = extract_bits(instr, 
				      regdp_format.M.index, 
				      regdp_format.M.bits); 
	//M-opr: type of instruction
	regdp_instr_t type = get_instr_type(opr | M << regdp_format.opr.bits);
	//last operand
	unsigned int operand = extract_bits(instr, 
					    regdp_format.operand.index, 
					    regdp_format.operand.bits); 
	//destination register
	unsigned int rd = extract_bits(instr, 
				       regdp_format.rd.index, 
				       regdp_format.rd.bits); 
	//register operand
	unsigned int rn = extract_bits(instr, 
				       regdp_format.rn.index, 
				       regdp_format.rn.bits); 
	//register for shift
	unsigned int rm = extract_bits(instr, 
				       regdp_format.rm.index, 
				       regdp_format.rm.bits); 
	//opcode
	unsigned int opc = extract_bits(instr, 
					regdp_format.opc.index, 
					regdp_format.opc.bits); 
	//first operand
	uint64_t op1 = 0;
	//second operand
	uint64_t op2 = 0; 
	//width depending on MSB
	int width = (extract_bits(instr, 
				  regdp_format.sf.index, 
				  regdp_format.sf.bits)) 
		    ? WIDTH_64 : WIDTH_32;

	int64_t result;
	
	//check that operand is in the valid range
	if (operand > (WIDTH_64 - 1) || 
           (type != MUL_INSTR && operand > width - 1)) {
		fprintf(stderr, "Invalid operand.\n");
		return EXIT_FAILURE;
	}
	assert(operand < WIDTH_64 && (type == MUL_INSTR || operand < width)); 

	//reads register rn into op1, if rn is not ZR
	if (rn != ZRSP) {
		if (read_reg(armv8, rn, &op1, width)) { return EXIT_FAILURE; }
	}

	//reads register rm into op2, if rm is not ZR
	if (rm != ZRSP) {
		if (read_reg(armv8, rm, &op2, width)) { return EXIT_FAILURE; }
	}

	//perform shift
	if(type != MUL_INSTR) {
		
		//Checks if the shift amount is within the valid range
		if (operand < 0 || operand > width - 1) {
			fprintf(stderr, "Invalid shift amount.\n");
			return EXIT_FAILURE;
		}
		assert(operand >= 0 && operand < width);

		shift_t shift = extract_bits(instr, 
					     regdp_format.shift.index, 
					     regdp_format.shift.bits);

		switch(shift){ //shift operand bits
			case LSL: //lsl
				op2 = logical_shift_left(armv8, 
							 op2, 
							 operand, 
							 width); 
				break;

			case LSR: //lsr
				op2 = logical_shift_right(armv8, 
							  op2, 
							  operand, 
							  width);
				break;

			case ASR: //asr
				op2 = arithmetic_shift_right(armv8, 
							     op2, 
							     operand, 
							     width);
				break;

			case ROR: //ror
				if (type == LOG_INSTR) {
					op2 = rotate_right(armv8, 
							   op2, 
							   operand, 
							   width);
				} else {
					fprintf(stderr, 
						"Unknown shift type" 
						"for arithmetic instruction\n");
					return EXIT_FAILURE;
				}
				break;
			
			default: 
			        fprintf(stderr, "Unknown shift type.\n");
				return EXIT_FAILURE;
		}
		
	}

	if(type == LOG_INSTR){
		//logical
		bool negate = extract_bits(instr, 
					   regdp_format.N.index, 
					   regdp_format.N.bits);
		
		if(negate) { op2 = ~op2; }

		switch(opc){ //specifies operation
			case LOG_AND: //and
				result = op1 & op2;
				break;

			case LOG_OR: //or
				result = op1 | op2;
				break;

			case LOG_XOR: //xor
				result = op1 ^ op2;
				break;

			case LOG_ANDS: //and, set flags
				result = op1 & op2;
				update_pstate(&armv8->PSTATE, 
					     op1, 
					     op2, 
					     result,
					     OP_LOGIC, 
					     width);
				break;

			default: 
				fprintf(stderr, 
					"Invalid operation code" 
					"for logical dp operation.\n");
				return EXIT_FAILURE;
		}

	}else if(type == ARITH_INSTR){ 
		//arithmetic
		result = perform_arithmetic(armv8, opc, op1, op2, width); 	

	}else if(type == MUL_INSTR){
		//multiply
		//madd if 0, msub if 1
		bool negate = extract_bits(instr, 
				           regdp_format.x.index, 
					   regdp_format.x.bits);
		//ra
		unsigned int ra = extract_bits(instr, 
					       regdp_format.ra.index, 
					       regdp_format.ra.bits); 

		//read in ra register, if ra is not ZR
		uint64_t op3 = 0;
		if (ra != ZRSP) {
			if (read_reg(armv8, ra, &op3, width)) { 
				return EXIT_FAILURE; 
			}
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
		fprintf(stderr, 
			"Unknown data processing register instruction\n");
		return EXIT_FAILURE;
	}
	
	// If zero register, do not write 
	if (rd == ZRSP) { return EXIT_SUCCESS; }

	return write_reg(armv8, rd, result, width);

}



