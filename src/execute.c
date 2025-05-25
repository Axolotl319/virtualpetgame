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
	int width = 32 + 32*(instr >> 31); //32 if sf = 0, 64 if sf = 1
	int opi = (instr >> 22) & 0x7;
	int opc = (instr >> 29) & 0x3;
	int rd = instr & 0xf;
	int result;

	switch(opi){
		case 2: //arithmetic
			int shift = 12 * ((instr >> 22) & 0x1);
			uint32_t imm = ((instr >> 10) & 0x7ff) << shift;
			int rn = (instr >> 5) & 0x1f;
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
			uint32_t imm = ((instr >> 5) & 0xffff) << shift;
			switch(opc){
				case 0: //move wide with NOT
					result = ~imm;
					break;

				case 2: //move wide with zero
					result = imm;
					break;

				case 3: //move wide with keep
					imm = imm >> shift;
					result = (armv8->GP_regs[rd]) & ~(0xff * shift); //set appropriate 16 bits to zero
					result = result | imm; //move imm into these 16 bits
					break;
			}
			break;

		default:
			fprintf(stderr, "Unknown OPI in immediate data processing instruction.");
			return 1;
			break;
	}


	if(width == 32){
		write_reg32(armv8, rd, result);
	}else{
		write_reg64(armv8, rd, result);
	}
	return 0;
}

void regdp(int instr) {}
void loadliteral(int instr) {}
void datatransfer(int instr) {}
void branch(int instr) {}
