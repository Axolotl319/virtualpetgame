#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"

// TODO -- the execution
void immdp(int instr) {
	int width = 32 + 32*(instr >> 31); //32 if sf = 0, 64 if sf = 1
	int opi = (instr >> 22) & 0x7;

	switch(opi){
		case 2: //arithmetic
			break;

		case 5: //wide move
			break;

		default:
			fprintf(stderr, "Unknown OPI in immediate data processing instruction.");
			return 1;
			break;
	}
}

void regdp(int instr) {}
void loadliteral(int instr) {}
void datatransfer(int instr) {}
void branch(int instr) {}
