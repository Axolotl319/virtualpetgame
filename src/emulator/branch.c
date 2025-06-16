#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "constants.h"
#include "armv8.h"
#include "emulator-utils.h"
#include "branch.h"
#include "instr-formats.h"

//sets PC if the condition holds, else returns failure
static int try_branch(bool cond, armv8_state *armv8, uint64_t new_addr) {
	if (cond) {
		if (setPC(armv8, new_addr)) { return BR_FAIL; }
		return BR_SUCCESS;
	}
	return BR_NOTHING;
}


//helper function for conditional branches
static int br_op_conditional(int cond, armv8_state *armv8, int64_t new_addr) {
	switch(cond) {
		case BR_EQ: //equal
			return try_branch(armv8->PSTATE.Z, armv8, new_addr);
			break;

		case BR_NE: //not equal
			return try_branch(!armv8->PSTATE.Z, armv8, new_addr);
			break;

		case BR_GE: //signed greater or equal 
			return try_branch(armv8->PSTATE.N == armv8->PSTATE.V,
					  armv8,
					  new_addr);
			break; 

		case BR_LT: //signed less than 
			return try_branch(armv8->PSTATE.N != armv8->PSTATE.V,
					  armv8,
					  new_addr);
			break; 

		case BR_GT: //signed greater than 
			return try_branch(!armv8->PSTATE.Z  && 
			    		  (armv8->PSTATE.N == armv8->PSTATE.V),
					  armv8,
					  new_addr);	
			break;

		case BR_LE: //signed less than or equal 
			return try_branch(!(!armv8->PSTATE.Z  && 
		    			  (armv8->PSTATE.N == armv8->PSTATE.V)),
					  armv8,
					  new_addr);
			break;

		case BR_AL: //always 
			return try_branch(1, armv8, new_addr);
			break;

		default: 
			fprintf(stderr, "Invalid condition code in branch.\n");
			return BR_FAIL; 
			break; 
		} 

	return BR_NOTHING;

}

// Input: integer representing an instruction 
// Based on the instruction, updates the PC to the desired address. 
// Returns 1 if branch success and condition met 
// Returns 0 if success but condition not met 
// Returns -1 in case of failure 
int branch(uint32_t instr, armv8_state *armv8) {
	unsigned int op = extract_bits(instr, 
			               br_format.op.index, 
				       br_format.op.bits);
        int32_t offset = 0; 
	int cond = 0; 	
	unsigned int reg = 0; 
	uint64_t addr = 0; 
	unsigned int simm26 = extract_bits(instr, 
			                   br_format.simm26.index, 
					   br_format.simm26.bits);

	unsigned int simm19 = extract_bits(instr, 
			                   br_format.simm19.index, 
					   br_format.simm19.bits);

	switch (op) {

	// Unconditional (offset) 
		case BR_OP_UNCONDITIONAL: 
			//Mask bits 26 onwards
			offset = (sign_ext_32(simm26, br_format.simm26.bits-1))
			       	 * WORD_SIZE_32;

			if (setPC(armv8, armv8->PC + offset)) { 
				return BR_FAIL; 
			}

			return BR_SUCCESS; 
			break; 

	// Conditional 
		case BR_OP_CONDITIONAL: 
			cond = extract_bits(instr, 
					    br_format.cond.index, 
					    br_format.cond.bits);

			//Mask bits 20 onwards
			offset = (sign_ext_32(simm19, br_format.simm19.bits-1))
			         * WORD_SIZE_32;
			
			// value to set PC
			uint64_t new_addr = armv8->PC + offset;

			// Determines which PSTATE flag to check
			return br_op_conditional(cond, armv8, new_addr);
		
			break; 

	// Unconditional (register)
		case BR_OP_REGISTER: 
			reg = extract_bits(instr, 
					   br_format.xn.index, 
					   br_format.xn.bits); 

		        // zero register, does not need to be handled
			if (reg != ZRSP && 
			    !read_reg(armv8, reg, &addr, WIDTH_64)) {	
				if (setPC(armv8, addr)) { return BR_FAIL; }
				return BR_SUCCESS;
			}
			break; 	

		default: 
			fprintf(stderr, "Invalid branch instruction.\n");
			return BR_FAIL; 
	} 

	return BR_NOTHING; 
}
