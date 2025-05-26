#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"
#include "modify-regs.h"
#include "armv8.h"

// TODO -- the execution
int perform_arithmetic(armv8_state *armv8, int opcode, int arg1, int arg2, int width){
	int result = INT_MAX;
	switch(opcode){
		case 0: //add
			result = arg1 + arg2;
			break;

		case 1: //adds
			result = arg1 + arg2;
			update_pstate(armv8->pstate, arg1, arg2, result, OP_ADD, width);
			break;

		case 2: //sub
			result = arg1 - arg2;
			break;

		case 3: //subs
			result = arg1 - arg2;
			update_pstate(armv8->pstate, arg1, arg2, result, OP_SUB, width);
			break;

		default:
			perror("Error. Unknown arithmetic opcode.");
			break;
	}
	return result;
}


void immdp(int instr, armv8_armstate *armv8) {
	unsigned int width = 32 * (instr >> 31);
	unsigned int opi = (instr >> 22) & 0x7;
	unsigned int opc = (instr >> 29) & 0x3;
	unsigned int rd = instr & 0x1f;
	int result;

	switch(opi){
		case 2: //arithmetic
			unsigned int shift = 12 * ((instr >> 22) & 0x1);
			int imm = ((instr >> 10) & 0xfff) << shift;
			unsigned int rn = (instr >> 5) & 0x1f;
			result = perform_arithmetic(armv8, opc, armv8->GP_regs[rn], imm, width);
			break;

		case 5: //wide move
			int shift = 16 * ((instr >> 21) & 0x3);
			int16_t imm = ((instr >> 5) & 0xffff);
			switch(opc){
				case 0: //move wide with NOT
					result = ~(imm << shift);
					break;

				case 2: //move wide with zero
					result = imm << shift;
					break;

				case 3: //move wide with keep
					result = (armv8->GP_regs[rd]) & ~(0xffff * shift); //set appropriate 16 bits to zero
					result = result | imm; //move imm into these 16 bits
					break;

				default:
					fprintf(stderr, "Unknown OPC for wide move instruction.");
					return 1;
			}
			break;

		default:
			fprintf(stderr, "Unknown OPI in immediate data processing instruction.");
			return 1;
			break;
	}


	if(width == 32){
		write_reg32(armv8, rd, result);
	}else{ //64-bit
		write_reg64(armv8, rd, result);
	}
	return 0;
}

void regdp(int instr, armv8_state *armv8) {
	unsigned int opr = (instr >> 21) & 0xffff;
	unsigned int type = opr | (((instr >> 28) & 0x1) << 3); //M-opr
	unsigned int operand = (instr >> 10) & 0x3f;
	unsigned int rd = instr & 0x1f;
	unsigned int rn = (instr >> 5) & 0x1f;
	unsigned int rm = (instr >> 16) & 0x1f;
	unsigned int opc = (instr >> 29) & 0x3;
	unsigned int width = 32 * ((instr >> 31) & 0x1);
	int result;

	if(type < 8){
		//logical
		bool negate = opr & 0x1;
		switch(opr & 0x6){ //shift bits
			case 0: //lsl
				logical_shift_left(armv8, rm, width); //to be defined in modify-regs.c (see declaration in header)
				break;

			case 1: //lsr
				logical_shift_right(armv8, rm, width); //to be defined in modify-regs.c (see declaration in header)
				break;

			case 2: //asr
				arithmetic_shift_right(armv8, rm, width); //to be defined in modify-regs.c (see declaration in header)
				break;

			case 3: //ror
				rotate_right(armv8, rm, width); //to be defined in modify-regs.c (see declaration in header)
				break;
		}
			
		if(negate){
			armv8->GP_regs[rm] = ~(armv8->GP_regs[rm])
		}

		switch(opc){ //specifies operation
			case 0: //and
				result = (armv8->GP_regs[rn]) & (armv8->GP_regs[rm]);
				break;

			case 1: //or
				result = (armv8->GP_regs[rn]) | (armv8->GP_regs[rm]);
				break;

			case 2: //xor
				result = (armv8->GP_regs[rn]) ^ (armv8->GP_regs[rm]);
				break;

			case 3: //and, set flags
				result = (armv8->GP_regs[rn]) & (armv8->GP_regs[rm]);
				update_pstate(armv8->pstate, armv8->GP_regs[rn], armv8->GP_regs[rm], result, OP_LOGIC, width);
				break;
			}

	}else if(type < 16 && type % 2 == 0){
		//arithmetic - might need to abstract for reg and imm
		switch(opr & 0x6){ //shift bits
			case 0: //lsl
				logical_shift_left(armv8, rm, width); //to be defined in modify-regs.c (see declaration in header)
				break;

			case 1: //lsr
				logical_shift_right(armv8, rm, width);
				break;

			case 2: //asr
				arithmetic_shift_right(armv8, rm, width);
				break;

			default:
				fprintf(stderr, "Unknown shift type for arithmetic dpr instruction.");
				return 1;
				break;
		}

		result = perform_arithmetic(armv8, opc, armv8->GP_regs[rn], imm, width);

	}else if(type == 24){
		//multiply
		bool negate = (instr >> 15) & 0x1; //madd if 0/false, msub if 1/true
		unsigned int ra = (instr >> 10) &0x1f;
		if(negate){
			result = armv8->GP_regs[ra] - (armv8->GP_regs[rn] * armv8->GP_regs[rm]);
		}else{
			result = armv8->GP_regs[ra] + (armv8->GP_regs[rn] * armv8->GP_regs[rm]);
		}
	}else{
		fprintf(stderr, "Unknown type of data processing register instruction.");
	}
	
	
	if(width == 0){ //sf = 0 -> 32-bit
		write_reg32(armv8, rd, result);
	}else{ //64-bit
		write_reg64(armv8, rd, result);
	}

}

void loadliteral(int instr, armv8_state *armv8) {}
void datatransfer(int instr, armv8_state *armv8) {}
void branch(int instr, armv8_state *armv8) {}
