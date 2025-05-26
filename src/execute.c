#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"

// TODO -- the execution
void immdp(int instr) {}
void regdp(int instr) {}
void loadliteral(int instr) {}
void datatransfer(int instr) {}

// Input: integer representing an instruction 
// Based on the instruction, updates the PC to the desired address. 
void branch(int instr, armv8_state *armv8) {
	int op = instr >> 30; 
	switch (op) {

	// Unconditional (offset) 
		case 0: 
			int offset = instr & 0x3ffffff; // Mask bits 26 onwards 
			// TODO: apply the offset to PC 
			break; 

	// Conditional 
		case 1: 
			int cond = instr & 0xf;
			int offset = (instr >> 5) & 0x13; // Mask bits 20 onwards 
			
			// Determines which PSTATE flag to check
			switch(cond) {
				case 0: 
					break; 
				case 1: 
					break; 
				case 10: 
					break; 
				case 11: 
					break; 
				case 12: 
					break; 
				case 13: 
					break; 
				case 14: 
					break; 
				default: 
					printf("Invalid condition code in branch.");
					break; 
			}
			break; 
	// Unconditional (register)
		case 3: 
			unsigned int reg = (instr >> 5) & 0x1f; 
		        break; 	
	} 
}
