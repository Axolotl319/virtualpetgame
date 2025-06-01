#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "sign-extension.h"
#include "data-transfer.h"
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

//Add read/write reg checks
static int load(armv8_state *armv8, int width, int rt, uint64_t addr){ 
	// uint8_t *addr = &(armv8->memory[transferAddress]); 
	// printf("Data to write: %p\n", addr); 
	uint64_t data;

	if (width == 32) {
		data = load32bit(armv8, addr);
	} else if (width == 64) {
		data = load64bit(armv8, addr);
	} else {
		fprintf(stderr, "Width must be 32 or 64 (load instruction)");
		return 1;
	}

	if (write_reg(armv8, rt, data, width)) { return 1; }

	return 0;
}

static int store(armv8_state *armv8, int width, int rt, uint64_t addr){
        //int32_t towrite1;
	uint64_t towrite;
	int word_limit;

	if (width == 32) {
		word_limit = 4;
	} else if (width == 64) {
		word_limit = 8;
	} else {
		fprintf(stderr, "Width must be 32 or 64 (store instruction)");
		return 1;
	}
	
	if (read_reg(armv8, rt, &towrite, width)) { return 1; }

	for (int i = 0; i < word_limit; i++) {
		armv8->memory[addr+i] = towrite & 0xff;
		towrite = towrite >> 8;
	}

	/*switch(width){
		case 32:
			if (read_reg32(armv8, rt, &towrite1)) { return 1; }

			for (int i=0; i<4; i++) {
				armv8->memory[addr+i] = towrite1 & 0xff; 
				towrite1 = towrite1 >> 8;
			}

			break;

		case 64:
			if (read_reg64(armv8, rt, &towrite2)) { return 1; }

			for (int i=0; i<8; i++) {
				armv8->memory[addr+i] = towrite2 & 0xff; 
				towrite2 = towrite2 >> 8; 
			}

			break;

		default:
			fprintf(stderr, "Width must be 32 or 64 (store instruction)");
			break;
	}*/
	return 0;
}

// Input: 32-bit instruction and pointer to armv8 state 
// Loads an immediate value into target register 
int loadliteral(int instr, armv8_state *armv8) {
	unsigned int reg = instr & 0x1f; 	// Obtains the target register
	int imm = sign_ext_32((instr >> 5) & 0x3ffff, 18)*4;	// Obtains the value to load
	int sf = (instr >> 30) & 0x1;		// Determines 32-bit or 64-bit
	
	int width;
	int64_t data;
	if (sf) {
		width = 64;
		data = load64bit(armv8, armv8->PC+imm);
	} else {
		width = 32;
		data = load32bit(armv8, armv8->PC+imm);
	}

	if (write_reg(armv8, reg, data, width)) { return 1; }

	return 0;
}

int datatransfer(int instr, armv8_state *armv8) {

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

	uint64_t transferAddress = 0;
	int rt = instr & 0x1f; //target register, contains data to store
			       //
	int xn = (instr >> 5) & 0x1f; //base register Xn
	if (xn == 0x1f) { return 0; } //Handle case when xn is the SP
	if (read_reg(armv8, xn, &transferAddress, 64)) { return 1; }  // Store base in transferAddress

	int width = ((instr >> 30) & 0x1) ? 64 : 32;
	
	// Pre-calculated for unsigned offset
	unsigned int offset = (instr >> 10) & 0x0fff; 

	// Pre-calculated for register offset
	int xm = (instr >> 16) & 0x1f; //index register offset
	uint64_t regoffset = 0; 		

	// Pre-calculated for pre/post index
	int32_t simm9 = sign_ext_32((instr >> 12) & 0x1ff, 9);

	assert(mode >= 0 && mode <= 3);
  
	//multiplier to calculate load size
	int multiplier; 

	switch(mode){
		case 0: //unsigned offset
			multiplier = (width == 32) ? 4 : 8;
			transferAddress += multiplier*offset;
			break;

		case 1: //register offset
			if (read_reg(armv8, xm, &regoffset, 64)) { return 1; } 
			transferAddress += regoffset;
			break;

		case 2: //pre-index
			transferAddress += simm9;
			if (write_reg(armv8, xn, transferAddress, width)) { return 1; }
			break;

		case 3: //post-index
			if (write_reg(armv8, xn, transferAddress + simm9, width)) { return 1;}
			break;
	}
 
	if((instr >> 22) & 0x1){ 
		if (load(armv8, width, rt, transferAddress)) { return 1; }
	} else{
		if (store(armv8, width, rt, transferAddress)) { return 1; }
	}

	return 0;
}

