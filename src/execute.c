#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"
#include "modify-regs.h"
#include "armv8.h"

// TODO -- the execution
void immdp(int instr, armv8_armstate *armv8) {
	int width = instr >> 31;
	int opi = (instr >> 22) & 0x7;
	int opc = (instr >> 29) & 0x3;
	unsigned int rd = instr & 0x1f;
	int result;

	switch(opi){
		case 2: //arithmetic
			int shift = 12 * ((instr >> 22) & 0x1);
			int imm = ((instr >> 10) & 0xfff) << shift;
			unsigned int rn = (instr >> 5) & 0x1f;
			switch(opc){
				case 0: //add Rd = Rn + Op2
					result = imm + armv8->GP_regs[rn];
					break;
				
				case 1: //adds Rd = Rn + Op2, set PSTATE
					result = imm + armv8->GP-regs[rn];
					update_pstate(armv8->pstate, armv8->GP_regs[rn], imm, result, OP_ADD, width);
					break;

				case 2: //sub Rd = Rn - Op2
					result = (armv8->GP_regs[rn]) - imm;
					break;

				case 3: //subs Rd = Rn - Op2, set PSTATE
					result = (armv8->GP_regs[rn]) - imm;
					update_pstate(armv8->pstate, armv8->GP_regs[rn], imm, result, OP_SUB, width);
					break;
			}
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


	if(width == 0){ //sf = 0 -> 32-bit
		write_reg32(armv8, rd, result);
	}else{ //64-bit
		write_reg64(armv8, rd, result);
	}
	return 0;
}

void regdp(int instr) {}
void loadliteral(int instr) {}
void datatransfer(int instr) {}
void branch(int instr) {}
