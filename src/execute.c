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



//Bitwise shift operations

//Shifts operand to the left, inserting zeros from least significant bit.
static uint64_t logical_shift_left(armv8_state *armv8, uint64_t operand, int shift, int width) {
    if (width == 32) {

		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
		return ((uint32_t)(operand) << shift);
    }
    return operand << shift;
}

//Shifts operand to the right, inserting zeros from most significant bit.
static uint64_t logical_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
    if (width == 32) {

		//The operand is masked and then shifted.
		//Return value is promoted to uint64_t to match function signature so no cast needed.
        return ((uint32_t)(operand) >> shift);
    }
    return operand >> shift;
}

//Operand shifted to the right and the most significant bit is copied into vacant positions.
static uint64_t arithmetic_shift_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
    if (width == 32) {
        uint32_t version32 = (uint32_t)operand;

        //Mod shift by 32 to ensure shift value is within the valid range 
        int32_t shifted32 = (int32_t)version32 >> (shift % 32);

        // Zero-extend back to 64 bits
        return (uint32_t)shifted32;
    } else {
        //Mod shift by 64 to ensure shift value is within the valid range
		//Casting to signed int32 to ensure sign-extension on >>
        int64_t shifted64 = (int64_t)operand >> (shift % 64);
        return (uint64_t)(shifted64);
    }
}

//Performs a bitwise right rotate on the operand, preserving all bits.
static uint64_t rotate_right(armv8_state *armv8, uint64_t operand, int shift, int width) {
	if (width == 32) {

		//Mod shift by 32 to ensure shift value is within the valid range 
		shift %= 32;
		uint32_t version32 = (uint32_t)operand;
		uint32_t rotated_bits = (version32 >> shift) | (version32 << (32 - shift));
		return rotated_bits;

	} else {
		//Mod shift by 63 to ensure shift value is within the valid range
		shift %= 64;
		uint64_t rotated_bits = (operand >> shift) | (operand << (64 - shift));
		return rotated_bits;
	}
}


// Performs arithmetic instructions
// Takes arguments: armv8 state pointer, opcode, 1st argument, 2nd argument, bit width
// Returns 64 bit result
static uint64_t perform_arithmetic(armv8_state *armv8, int opcode, unsigned int arg1, unsigned int arg2, int width){
	uint64_t result = UINT64_MAX;

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
	return result;
}

//Immediate data processing instructions
//Takes arguments: 32 bit instruction, armv8 state pointer
//Returns 0 for success, 1 for failure
int immdp(uint32_t instr, armv8_state *armv8) {
	unsigned int width = ((instr >> 31) & 0x1) ? 64 : 32; //MSB = 1: Width = 64, MSB = 0: Width = 32
	unsigned int opi = (instr >> 23) & 0x7; //Data processing operation 010=Arithmetic, 101=Wide move
	unsigned int opc = (instr >> 29) & 0x3; //Operation code
	unsigned int rd = instr & 0x1f; //Destination register
	uint64_t result;

	switch(opi){
		case 2: { //arithmetic
			unsigned int shift = ((instr >> 22) & 0x1) ? 12 : 0; //shift by 12 if bit 22 = 1 
			uint64_t imm = ((instr >> 10) & 0xfff) << shift; //shifted immediate value 
			unsigned int rn = (instr >> 5) & 0x1f; //1st operand register
			
			//performs arithmetic based on bit width
			uint64_t op = 0;
			int status = (width == 32) ? read_reg32(armv8, rn, (uint32_t *)&op) : read_reg64(armv8, rn, &op);
		        if (status) return 1;	

			result = (width == 32) ? 
				perform_arithmetic(armv8, opc, (uint32_t) op, (uint32_t) imm, width) :
				perform_arithmetic(armv8, opc, op, imm, width);

			break;
		}

		case 5: { //wide move
			int hw = ((instr >> 21) & 0x3);
			
			if (width == 32 && hw > 1) {
				fprintf(stderr, "Invalid shift for 32 bit");
				return 1;
			}
			int shift = 16 * hw;

			uint64_t imm = ((uint64_t)((instr >> 5) & 0xffff)) << shift; // shifted immediate value
			switch(opc){
				case 0: //move wide with NOT
					result = ~(imm);
					break;
				
				case 2: //move wide with zero
					result = imm;
					break;

				case 3: { //move wide with keep
					
					//reads rd register 	
					uint64_t value;
                        		if (read_reg64(armv8, rd, &value)) return 1;

					//Masking bits
					result = (value) & ~((uint64_t)0xffff << shift); //set appropriate 16 bits to zero
					result = result | imm; //move imm into these 16 bits
					break;
				}

				default:
					fprintf(stderr, "Unknown OPC for wide move instruction.");
					return 1;
			}
			break;
		}

		default: {
			fprintf(stderr, "Unknown OPI in immediate data processing instruction.");
			return 1;
			break;
		}
	}
	
	// If destination is 0 register, do not write. 
	if (rd == 0x1f) return 0;

	if(width == 32){ //32-bit
		return write_reg32(armv8, rd, result);
	}else{ //64-bit
		return write_reg64(armv8, rd, result);
	}

}


int regdp(uint32_t instr, armv8_state *armv8) {
	unsigned int opr = (instr >> 21) & 0xf; //opr = bits 21-24 of instruction
	unsigned int type = opr | (((instr >> 28) & 0x1) << 4); //M-opr: type of instruction (M = bit 28)
	unsigned int operand = (instr >> 10) & 0x3f; //last operand
	unsigned int rd = instr & 0x1f; //destination register
	unsigned int rn = (instr >> 5) & 0x1f; //register operand
	unsigned int rm = (instr >> 16) & 0x1f; //register that shift is performed on
	unsigned int opc = (instr >> 29) & 0x3; //opcode
	uint64_t op1 = 0; //first operand
	uint64_t op2 = 0; //second operand
	int width = ((instr >> 31) & 0x1) ? 64 : 32; //width depending on MSB
	uint64_t result;
	
	//check that operand is in the valid range
	if (operand > 63 || (type != 24 && (width == 32 && operand > 31))) {
		fprintf(stderr, "Invalid operand");
		return 1;
	}

	//reads register rn into op1, if rn is not ZR
	if (rn != 0x1f) {
		int status = (width == 32) ? read_reg32(armv8, rn, (uint32_t *)&op1) : read_reg64(armv8, rn, &op1);	
        	if (status) return 1;
	}

	//reads register rm into op2, if rm is not ZR
	if (rm != 0x1f) {
		int status = (width == 32) ? read_reg32(armv8, rm, (uint32_t *)&op2) : read_reg64(armv8, rm, &op2);
        	if (status) return 1;
	}

	//perform shift
	if(type != 24) {
		
		switch((opr >> 1) & 0x3){ //shift op2 bits
			case 0: //lsl
				op2 = logical_shift_left(armv8, op2, operand, width); //defined in modify-regs.c 
				break;

			case 1: //lsr
				op2 = logical_shift_right(armv8, op2, operand, width);
				break;

			case 2: //asr
				op2 = arithmetic_shift_right(armv8, op2, operand, width);
				break;

			case 3: //ror
				if (type < 8) {
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

	if(type < 8){
		//logical
		bool negate = opr & 0x1;
		
		if(negate){
			op2 = ~op2;
		}

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
		}

	}else if(type < 16 && type % 2 == 0){ 
		//arithmetic 
			result = (width == 32) ?
		        perform_arithmetic(armv8, opc, (uint32_t) op1, (uint32_t) op2, width) :	
			perform_arithmetic(armv8, opc, op1, op2, width); 
					    
	}else if(type == 24){
		//multiply
		bool negate = (operand >> 5) & 0x1; //madd if 0/false, msub if 1/true
		unsigned int ra = operand & 0x1f;

		//read in ra register, if ra is not ZR
		uint64_t op3 = 0;
		if (ra != 0x1f) {
			int status = (width == 32) ? read_reg32(armv8, ra, (uint32_t *) &op3) : read_reg64(armv8, ra, &op3);
			if (status) return 1;
		}

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
	if (rd == 0x1f) {
		return 0; 
	}

	if(width == 32){ 
		return write_reg32(armv8, rd, result);
	}else{ //64-bit
		return write_reg64(armv8, rd, result);
	}

}

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
	
	switch(width){
		case 32:
			;
			uint32_t towrite1 = 0; 
			read_reg32(armv8, rt, &towrite1);
			for (int i=0; i<4; i++) {
				armv8->memory[addr+i] = towrite1 & 0xf; 
				towrite1 = towrite1 >> 4; 
			}
			break;

		case 64:
			;
			uint64_t towrite2 = 0; 
			read_reg64(armv8, rt, &towrite2);
			for (int i=0; i<8; i++) {
				armv8->memory[addr+i] = towrite2 & 0xf; 
				towrite2 = towrite2 >> 4; 
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
	int imm = ((instr >> 5) & 0x13)*4;	// Obtains the value to load
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

	uint64_t transferAddress = 0;
	int rt = instr & 0x1f; //target register, contains data to store
	int xn = (instr >> 5) & 0x1f; //base register Xn
	read_reg64(armv8, xn, &transferAddress);  // Store base in transferAddress 
	int width = ((instr >> 30) & 0x1f) ? 64 : 32;
	
	// Pre-calculated for unsigned offset
	unsigned int offset = (instr >> 10) & 0x0fff; 

	// Pre-calculated for register offset
	int xm = (instr >> 16) & 0x1f; //index register offset
	uint64_t regoffset = 0; 		

	// Pre-calculated for pre/post index
	int simm9 = (instr >> 12) & 0x1ff;

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
			transferAddress += regoffset;
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
        int64_t offset = 0; 
	int cond = 0; 	
	unsigned int reg = 0; 
	uint64_t addr = 0; 
	switch (op) {

	// Unconditional (offset) 
		case 0: 
			offset = (instr & 0x3ffffff)*4; // Mask bits 26 onwards 
			setPC(armv8, armv8->PC + offset); 
			return 1; 
			break; 

	// Conditional 
		case 1: 
			cond = instr & 0xf;
			offset = ((instr >> 5) & 0x7ffff)*4; // Mask bits 20 onwards 
			
			// Determines which PSTATE flag to check
			switch(cond) {
				case 0: 
					if (armv8->PSTATE.Z == true) {
						setPC(armv8, armv8->PC + offset); 
						return 1;
					}
					break; 
				case 1: 
					if (armv8->PSTATE.Z == false) {
						setPC(armv8, armv8->PC + offset); 
						return 1; 
					}
					break; 
				case 10: 
					if (armv8->PSTATE.N == armv8->PSTATE.V) {
						setPC(armv8, armv8->PC + offset); 
						return 1; 
					}
					break; 
				case 11: 
					if (armv8->PSTATE.N != armv8->PSTATE.V) {
						setPC(armv8, armv8->PC + offset); 
						return 1; 
					}
					break; 
				case 12: 
					if (armv8->PSTATE.Z == false && armv8->PSTATE.N == armv8->PSTATE.Z) {
						setPC(armv8, armv8->PC + offset); 
						return 1; 
					}
					break; 
				case 13: 
					if (!(armv8->PSTATE.Z == false && armv8->PSTATE.N == armv8->PSTATE.Z)) {
						setPC(armv8, armv8->PC + offset); 
						return 1; 
					}
					break; 
				case 14: 
					setPC(armv8, armv8->PC + offset); 
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
				setPC(armv8, addr);
				return 1; 
			}
			break; 	
		default: 
			printf("Invalid branch instruction.");
			return -1; 
	} 
	return 0; 
}
