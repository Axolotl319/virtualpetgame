#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "sign-extension.h"
#include "branch.h"
#include "modify-regs.h"
#include <limits.h>
#include <assert.h>


// Input: integer representing an instruction 
// Based on the instruction, updates the PC to the desired address. 
// Returns 1 if branch success and condition met 
// Returns 0 if success but condition not met 
// Returns -1 in case of failure 
int branch(int instr, armv8_state *armv8) {
	unsigned int op = (instr >> 30) & 0x3;
        int32_t offset = 0; 
	int cond = 0; 	
	unsigned int reg = 0; 
	int64_t addr = 0; 
	switch (op) {

	// Unconditional (offset) 
		case 0: 
			offset = (sign_ext_32(instr & 0x2ffffff, 25))*4; // Mask bits 26 onwards 
			if (setPC(armv8, armv8->PC + offset)) { return -1; }
			return 1; 
			break; 

	// Conditional 
		case 1: 
			cond = instr & 0xf;
			offset = (sign_ext_32((instr >> 5) & 0x3ffff, 18))*4; // Mask bits 20 onwards 
			printf("This is the offset calculated: %08x\n", offset); 
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
					printf("Invalid condition code in branch.");
					return -1; 
					break; 
			}
			return 0; 
			break; 
	// Unconditional (register)
		case 3: 
			reg = (instr >> 5) & 0x1f; 
		        // 0x1f is the zero register, does not need to be handled
			if (reg != 0x1f && read_reg64(armv8, reg, &addr) == 0) {
				if (setPC(armv8, (unsigned int)addr)) { return -1; }
				return 1; 
			}
			break; 	
		default: 
			printf("Invalid branch instruction.");
			return -1; 
	} 
	return 0; 
}
