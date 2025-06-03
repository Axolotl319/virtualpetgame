#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "sign-extension.h"
#include "branch.h"
#include "modify-regs.h"
#include "extract-bits.h"
#include "instr-formats.h"
#include "constants.h"

// Input: integer representing an instruction 
// Based on the instruction, updates the PC to the desired address. 
// Returns 1 if branch success and condition met 
// Returns 0 if success but condition not met 
// Returns -1 in case of failure 
int branch(uint32_t instr, armv8_state *armv8) {
	unsigned int op = extract_bits(instr, br_format.op.index, br_format.op.bits);
        int32_t offset = 0; 
	int cond = 0; 	
	unsigned int reg = 0; 
	uint64_t addr = 0; 
	unsigned int simm26 = extract_bits(instr, br_format.simm26.index, br_format.simm26.bits);
	unsigned int simm19 = extract_bits(instr, br_format.simm19.index, br_format.simm19.bits);

	switch (op) {

	// Unconditional (offset) 
		case 0: 
			offset = (sign_ext_32(simm26, br_format.simm26.bits - 1)) * WORD_SIZE; // Mask bits 26 onwards 
			if (setPC(armv8, armv8->PC + offset)) { return -1; }
			return 1; 
			break; 

	// Conditional 
		case 1: 
			cond = extract_bits(instr, br_format.cond.index, br_format.cond.bits);
			offset = (sign_ext_32(simm19, br_format.simm19.bits - 1)) * WORD_SIZE; // Mask bits 20 onwards 

			// Determines which PSTATE flag to check
			switch(cond) {
				case 0: 
					if (armv8->PSTATE.Z == true) {
						if (setPC(armv8, armv8->PC + offset)) { return -1; } 
						return 1;
					}
					break;	
				case 1: 
					if (armv8->PSTATE.Z == false) {
						if (setPC(armv8, armv8->PC + offset)) { return -1; } 
						return 1; 
					}
					break; 
				case 10: 
					if (armv8->PSTATE.N == armv8->PSTATE.V) {
						if (setPC(armv8, armv8->PC + offset)) { return -1; } 
						return 1; 
					}
					break; 
				case 11: 
					if (armv8->PSTATE.N != armv8->PSTATE.V) {
						if (setPC(armv8, armv8->PC + offset)) { return -1; } 
						return 1; 
					}
					break; 
				case 12: 
					if (armv8->PSTATE.Z == false && armv8->PSTATE.N == armv8->PSTATE.Z) {
						if (setPC(armv8, armv8->PC + offset)) { return -1; } 
						return 1; 
					}
					break; 
				case 13: 
					if (!(armv8->PSTATE.Z == false && armv8->PSTATE.N == armv8->PSTATE.Z)) {
						if (setPC(armv8, armv8->PC + offset)) { return -1; } 
						return 1; 
					}
					break; 
				case 14: 
					if (setPC(armv8, armv8->PC + offset)) { return -1; } 
					return 1; 
					break; 
				default: 
					fprintf(stderr, "Invalid condition code in branch.");
					return -1; 
					break; 
			}
			return 0; 
			break; 

	// Unconditional (register)
		case 3: 
			reg = extract_bits(instr, br_format.xn.index, br_format.xn.bits); 
		        // zero register, does not need to be handled
			if (reg != ZRSP && read_reg(armv8, reg, &addr, WIDTH_64) == 0) {
				if (setPC(armv8, addr)) { return -1; }
				return 1; 
			}
			break; 	
		default: 
			fprintf(stderr, "Invalid branch instruction.");
			return -1; 
	} 
	return 0; 
}
