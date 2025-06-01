#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "execute.h"
#include "modify-regs.h"
#include <limits.h>
#include <assert.h>

static uint32_t load32bit(armv8_state *armv8, uint64_t addr) {
	 return ((uint32_t)armv8->memory[addr])
		| ((uint32_t)armv8->memory[addr + 1] << 8) 
		| ((uint32_t)armv8->memory[addr + 2] << 16) 
		| ((uint32_t)armv8->memory[addr + 3] << 24);			
}	

static uint64_t load64bit(armv8_state *armv8, uint64_t addr) {
	return ((uint64_t)armv8->memory[addr]) 
		| ((uint64_t)armv8->memory[addr + 1] << 8)
		| ((uint64_t)armv8->memory[addr + 2] << 16)
		| ((uint64_t)armv8->memory[addr + 3] << 24)
		| ((uint64_t)armv8->memory[addr + 4] << 32)
		| ((uint64_t)armv8->memory[addr + 5] << 40)
		| ((uint64_t)armv8->memory[addr + 6] << 48)
		| ((uint64_t)armv8->memory[addr + 7] << 56);		
}

static int32_t sign_ext_32(int num, int msb_num) { 
	if (num >> (msb_num-1)) {
		printf("Resulting calculation: %08x\n", num | (0xffffffff << msb_num)); 
		return num | (0xffffffff << msb_num);
	}
	return num;
}

//Add read/write reg checks
static void load(armv8_state *armv8, int width, int rt, uint64_t addr){ 
	// uint8_t *addr = &(armv8->memory[transferAddress]); 
	// printf("Data to write: %p\n", addr); 
 
	switch(width){
		case 32:
			;
			uint32_t data1 = load32bit(armv8, addr);
			write_reg32(armv8, rt, data1);
			break;

		case 64:
			;
			uint64_t data2 = load64bit(armv8, addr);
			printf("Data to write: %lu\n", data2); 
			write_reg64(armv8, rt, data2);
			break;

		default:
			perror("Width must be 32 or 64 (load instruction)");
			break;
	}
}

static void store(armv8_state *armv8, int width, int rt, uint64_t addr){
	printf("Address to write to: %lx\n", addr); 
	switch(width){
		case 32:
			;
			int32_t towrite1 = 0;	
			read_reg32(armv8, rt, &towrite1);
			printf("Data to store: %x\n", towrite1); 
			for (int i=0; i<4; i++) {
				armv8->memory[addr+i] = towrite1 & 0xff; 
				towrite1 = towrite1 >> 8;
			        printf("Remaining bytes to write: %x\n", towrite1); 	
			}
			break;

		case 64:
			;
			int64_t towrite2 = 0; 
			read_reg64(armv8, rt, &towrite2);
			printf("Data to store: %lx\n", towrite2); 
			for (int i=0; i<8; i++) {
				armv8->memory[addr+i] = towrite2 & 0xff; 
				towrite2 = towrite2 >> 8; 
			}
			break;

		default:
			perror("Width must be 32 or 64 (store instruction)");
			break;
	}
}

// Input: 32-bit instruction and pointer to armv8 state 
// Loads an immediate value into target register 
void loadliteral(int instr, armv8_state *armv8) {
	unsigned int reg = instr & 0x1f; 	// Obtains the target register
	int imm = sign_ext_32((instr >> 5) & 0x3ffff, 18)*4;	// Obtains the value to load
	int sf = (instr >> 30) & 0x1;		// Determines 32-bit or 64-bit
	if (sf) {				// If sf == 1, 64-bit
		write_reg64(armv8, reg, load64bit(armv8, armv8->PC+imm)); 
	} else {				// Otherwise, 32-bit 
		write_reg32(armv8, reg, load32bit(armv8, armv8->PC+imm)); 
	}	
}

void datatransfer(int instr, armv8_state *armv8) {

	int mode; //represents addressing mode
	if(((instr >> 24) & 0x1) == 1){
		mode = 0; //unsigned offset
	}else if(((instr >> 21) & 0x1) == 1){
		mode = 1; //register offset
	}else if(((instr >> 11) & 0x1) == 1){
		mode = 2; //pre-index
	}else{
		mode = 3; //post-index
	}

	int64_t transferAddress = 0;
	int rt = instr & 0x1f; //target register, contains data to store
	int xn = (instr >> 5) & 0x1f; //base register Xn
	read_reg64(armv8, xn, &transferAddress);  // Store base in transferAddress 
	int width = ((instr >> 30) & 0x1) ? 64 : 32;
	
	// Pre-calculated for unsigned offset
	unsigned int offset = (instr >> 10) & 0x0fff; 

	// Pre-calculated for register offset
	int xm = (instr >> 16) & 0x1f; //index register offset
	int64_t regoffset = 0; 		

	// Pre-calculated for pre/post index
	int32_t simm9 = sign_ext_32((instr >> 12) & 0x1ff, 9);

	assert(mode >= 0 && mode <= 3);
        printf("Transfer address: %lu\n", transferAddress); 	
	switch(mode){
		case 0: //unsigned offset
			switch (width){
				case 32:
					transferAddress += 4*offset;
					break;

				case 64:
					transferAddress += 8*offset;
					break;
			}
			break;

		case 1: //register offset
			read_reg64(armv8, xm, &regoffset); 
			transferAddress += (unsigned int) regoffset;
			break;

		case 2: //pre-index
			transferAddress += simm9;
			switch(width){
				case 32:
					write_reg32(armv8, xn, transferAddress);
					break;

				case 64:
					write_reg64(armv8, xn, transferAddress);
					break;
			}
			break;

		case 3: //post-index
			switch(width){
				case 32:
					write_reg32(armv8, xn, transferAddress + simm9);
					break;

				case 64:
					write_reg64(armv8, xn, transferAddress + simm9);
					break;
			}
			break;
	}
 
	if((instr >> 22) & 0x1){ 
		load(armv8, width, rt, transferAddress);
	} else{
		store(armv8, width, rt, transferAddress);
	}
}

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
