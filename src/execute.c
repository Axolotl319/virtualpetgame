#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"
#include "modify-regs.h"

// TODO -- the execution
void immdp(int instr) {}
void regdp(int instr) {}
void loadliteral(int instr) {}
void datatransfer(int instr) {}

// Input: integer representing an instruction 
// Based on the instruction, updates the PC to the desired address. 
void branch(int instr, armv8_state *armv8) {
	int op = instr >> 30;
        int64_t offset = 0; 
	int cond = 0; 	
	unsigned int reg = 0; 
	uint64_t *addr = 0; 
	switch (op) {

	// Unconditional (offset) 
		case 0: 
			offset = (instr & 0x3ffffff)*4; // Mask bits 26 onwards 
			printf("%lu\n", armv8->PC);
			setPC(armv8, armv8->PC + offset);
		        printf("After: %lu\n", armv8->PC); 	
			break; 

	// Conditional 
		case 1: 
			cond = instr & 0xf;
			offset = ((instr >> 5) & 0x13)*4; // Mask bits 20 onwards 
			
			// Determines which PSTATE flag to check
			switch(cond) {
				case 0: 
					if (armv8->PSTATE.Z == true) {
						setPC(armv8, armv8->PC + offset); 
					}
					break; 
				case 1: 
					if (armv8->PSTATE.Z == false) {
						setPC(armv8, armv8->PC + offset); 
					}
					break; 
				case 10: 
					if (armv8->PSTATE.N == armv8->PSTATE.V) {
						setPC(armv8, armv8->PC + offset); 
					}
					break; 
				case 11: 
					if (armv8->PSTATE.N != armv8->PSTATE.V) {
						setPC(armv8, armv8->PC + offset); 
					}
					break; 
				case 12: 
					if (armv8->PSTATE.Z == false && armv8->PSTATE.N == armv8->PSTATE.Z) {
						setPC(armv8, armv8->PC + offset); 
					}
					break; 
				case 13: 
					if (!(armv8->PSTATE.Z == false && armv8->PSTATE.N == armv8->PSTATE.Z)) {
						setPC(armv8, armv8->PC + offset); 
					}
					break; 
				case 14: 
					setPC(armv8, armv8->PC + offset); 
					break; 
				default: 
					printf("Invalid condition code in branch.");
					break; 
			}
			break; 
	// Unconditional (register)
		case 3: 
			reg = (instr >> 5) & 0x1f; 
		        // 0x1f is the zero register, does not need to be handled
			if (reg != 0x1f) {
				read_reg64(armv8, reg, addr); 
				setPC(armv8, *addr); 	
			}
			break; 	
		default: 
			printf("Invalid branch instruction.");
	} 
}
