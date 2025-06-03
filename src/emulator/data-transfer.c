#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "sign-extension.h"
#include "data-transfer.h"
#include "modify-regs.h"
#include "extract-bits.h"
#include "instr-formats.h"
#include "constants.h"
#include <limits.h>
#include <assert.h>

//Loads value from memory
//Takes arguments: armv8 pointer, mem address, data pointer
//Returns 0 if success, 1 if failure
static int load32bit(armv8_state *armv8, uint64_t addr, uint32_t *data) {
	 //Bounds checking
	 if (addr + 3 >= MEM_SIZE) {
		 fprintf(stderr, "Invalid memory address for load instruction");
		 return 1;
	 }

	 *data = ((uint32_t)armv8->memory[addr])
		| ((uint32_t)armv8->memory[addr + 1] << 8) 
		| ((uint32_t)armv8->memory[addr + 2] << 16) 
		| ((uint32_t)armv8->memory[addr + 3] << 24);	
	
	 return 0;	 
}	

static int load64bit(armv8_state *armv8, uint64_t addr, uint64_t *data) {
	//Bounds checking
	if (addr + 7 >= MEM_SIZE) {
		fprintf(stderr, "Invalid memory addresss for load instruction");
		return 1;
	}
	
	*data = ((uint64_t)armv8->memory[addr]) 
		| ((uint64_t)armv8->memory[addr + 1] << 8)
		| ((uint64_t)armv8->memory[addr + 2] << 16)
		| ((uint64_t)armv8->memory[addr + 3] << 24)
		| ((uint64_t)armv8->memory[addr + 4] << 32)
		| ((uint64_t)armv8->memory[addr + 5] << 40)
		| ((uint64_t)armv8->memory[addr + 6] << 48)
		| ((uint64_t)armv8->memory[addr + 7] << 56);
	
	return 0;	
}

//Add read/write reg checks
static int load(armv8_state *armv8, int width, int rt, uint64_t addr){ 
	uint64_t data;

	if (width == WIDTH_32) {
		if (load32bit(armv8, addr, (uint32_t *)&data)) { 
			return 1;
		}

	} else if (width == WIDTH_64) {
		if (load64bit(armv8, addr, &data)) { 
			return 1; 
		}

	} else {
		fprintf(stderr, "Width must be 32 or 64 (load instruction)");
		return 1;
	}

	if (write_reg(armv8, rt, data, width)) { return 1; }

	return 0;
}

static int store(armv8_state *armv8, int width, int rt, uint64_t addr){
	uint64_t towrite;
	int word_limit;

	if (width == WIDTH_32) {
		word_limit = WORD_SIZE_32;
	} else if (width == WIDTH_64) {
		word_limit = WORD_SIZE_64;
	} else {
		fprintf(stderr, "Width must be 32 or 64 (store instruction)");
		return 1;
	}

	//bounds checking
	if (addr + word_limit > MEM_SIZE) { 
		return 1; 
	}
	
	if (read_reg(armv8, rt, &towrite, width)) { return 1; }

	for (int i = 0; i < word_limit; i++) {
		armv8->memory[addr+i] = towrite & MASK_8;
		towrite >>= WORD_SIZE_64;
	}
	return 0;
}

// Input: 32-bit instruction and pointer to armv8 state 
// Loads an immediate value into target register 
int loadliteral(uint32_t instr, armv8_state *armv8) {
	unsigned int reg = extract_bits(instr, sdt_format.rt.index, sdt_format.rt.bits); 	// Obtains the target register
	unsigned int simm19 = extract_bits(instr, sdt_format.simm19.index, sdt_format.simm19.bits);
	int imm = sign_ext_32(simm19, sdt_format.simm19.bits) * WORD_SIZE_32;	// Obtains the value to load
	int sf = extract_bits(instr, sdt_format.sf.index, sdt_format.sf.bits);		// Determines 32-bit or 64-bit
	
	int width;
	uint64_t data;
	if (sf) {
		width = WIDTH_64;
		if (load64bit(armv8, armv8->PC+imm, &data)) { return 1; }
	} else {
		width = WIDTH_32;
		if (load32bit(armv8, armv8->PC+imm, (uint32_t *) &data)) { return 1; }
	}

	if (write_reg(armv8, reg, data, width)) { return 1; }

	return 0;
}

int datatransfer(uint32_t instr, armv8_state *armv8) {
	int mode = MODE_POST_INDEX; //represents addressing mode
	if(extract_bits(instr, sdt_format.U.index, sdt_format.U.bits)){
		mode = MODE_UNSIGNED_OFFSET; //unsigned offset
	}else if(extract_bits(instr, sdt_format.R.index, sdt_format.R.bits)){
		mode = MODE_REG_OFFSET; //register offset
	}else if(extract_bits(instr, sdt_format.I.index, sdt_format.I.bits)){
		mode = MODE_PRE_INDEX; //pre-index
	} else {
		mode = MODE_POST_INDEX;
	}

	uint64_t transferAddress = 0;
	int rt = extract_bits(instr, sdt_format.rt.index, sdt_format.rt.bits); //target register, contains data to store
	int xn = extract_bits(instr, sdt_format.xn.index, sdt_format.xn.bits); //base register Xn
	if (xn == ZRSP) { return 0; } //Handle case when xn is the SP
	if (read_reg(armv8, xn, &transferAddress, WIDTH_64)) { return 1; }  // Store base in transferAddress

	int width = extract_bits(instr, sdt_format.sf.index, sdt_format.sf.bits) ? WIDTH_64 : WIDTH_32;

	// Pre-calculated for unsigned offset
	unsigned int offset = extract_bits(instr, sdt_format.offset.index, sdt_format.offset.bits); 

	// Pre-calculated for register offset
	int xm = extract_bits(instr, sdt_format.xm.index, sdt_format.xm.bits); //index register offset
	uint64_t regoffset = 0; 		

	// Pre-calculated for pre/post index
	int32_t simm9 = sign_ext_32(extract_bits(instr, sdt_format.simm9.index, sdt_format.simm9.bits), sdt_format.simm9.bits);

	assert(mode >= MODE_UNSIGNED_OFFSET && mode <= MODE_POST_INDEX);
  
	//multiplier to calculate load size
	int multiplier; 

	switch(mode){
		case MODE_UNSIGNED_OFFSET: //unsigned offset
			multiplier = (width == WIDTH_32) ? WORD_SIZE_32 : WORD_SIZE_64;
			transferAddress += multiplier*offset;
			break;

		case MODE_REG_OFFSET: //register offset
			if (read_reg(armv8, xm, &regoffset, WIDTH_64)) { return 1; } 
			transferAddress += regoffset;
			break;

		case MODE_PRE_INDEX: //pre-index
			transferAddress += simm9;
			if (write_reg(armv8, xn, transferAddress, width)) { return 1; }
			break;

		case MODE_POST_INDEX: //post-index
			if (write_reg(armv8, xn, transferAddress + simm9, width)) { return 1;}
			break;
	}
 
	if(extract_bits(instr, sdt_format.L.index, sdt_format.L.bits)){ 
		if (load(armv8, width, rt, transferAddress)) { return 1; }
	} else{
		if (store(armv8, width, rt, transferAddress)) { return 1; }
	}

	return 0;
}
