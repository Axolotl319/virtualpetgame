#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "symtable.h"
#include "data-transfer.h"
#include "assembly-utils.h"
#include "constants.h"

#define MAX_PARAMS 5
#define MIN_PARAMS 3

// Return encoded instruction if success, -1 if fail
int dt(char **params, int numparams) {
	printf("debug: this is a data transfer instruction\n");
	uint32_t toReturn;
	if(numparams > MAX_PARAMS || numparams < MIN_PARAMS){
		printf("debug: incorrect no. params\n");
		fprintf(stderr, "Unexpected no. parameters for dt instr\n");
		return -1;
	}
	char *type = params[0]; // load/store instruction
	char *target = params[1]; // first argument is target register
	uint8_t rt = obtain_reg_num(target);

	bool loadLiteral = (numparams == 3) & (*params[2] != 'x');
	//load literal is ldr with 2 args, sdts have an extra argument
	//unsigned offset can also just have 3 params, the 3rd being a Xn reg
	
	if(loadLiteral){
		printf("debug: this is a load literal\n");
		toReturn = 0x18000000;

		char *value = params[2]; //#imm or label offset
		uint32_t simm19;
		if(*value == '#'){
			//immediate value
			simm19 = extract_imm(value);
		}else{
			//value is a label offset (label addr - curr addr)
			simm19 = strtol(value, NULL, 16);
		}
		printf("debug: simm19 = %x\n", simm19);
		toReturn |= (simm19 << 5); //set bits 5-23 with simm19 value
		printf("debug: toReturn with simm19 = %x\n", toReturn);
	}else if(strcmp(type, "ldr") == 0){
		//load instruction, no load literal
		toReturn = 0xb9400000; //L bit set
	}else if(strcmp(type, "str") == 0){
		//store instruction
		toReturn = 0xb9000000; //L bit not set
	}else{
		fprintf(stderr, "Data transfer instruction is not ldr or str\n");
		return 0;
	}

	if(!loadLiteral){
		int mode;
		uint8_t xn = param[2];
		if(numparams == 3){ mode = MODE_UNSIGNED_OFFSET; } //Zero Unsigned Offset
	}

	update_sf(&toReturn, 30, target); //update register width based on target register
	toReturn |= rt; //replace last 5 bits with target reg number
	printf("debug: toReturn = %x\n", toReturn);

	return toReturn; 
}
