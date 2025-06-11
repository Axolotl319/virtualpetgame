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

static char *removeBrackets(char *str){
	char *without = strtok(str, "[ ]");
	return without;
}

static bool is_imm(char *value){
	return (*value == '#');
}

//returns addressing mode or -1 if error occurs
static int mode(char **params, int numparams){
	if(numparams < 3){
		fprintf(stderr, "Unknown addressing mode");
		return -1;
	}
	if(numparams == 3){ return MODE_UNSIGNED_OFFSET; }
	char last_char = *(params[3] + (strlen(params[3]) - 2));
	if(last_char == '!'){ return MODE_PRE_INDEX; }
	if(last_char == ']'){
		if(is_imm(params[3])){
			return MODE_UNSIGNED_OFFSET;
		}else{
			return MODE_REG_OFFSET;
		}
	}
	//if no previous conditions were met
	return MODE_POST_INDEX;
}

//returns imm12 (when it isn't 0) or 0 if there's an error
static int getImm12(char *imm_str, char *reg){
	int imm = atoi(strtok(imm_str, "[ ]#"));
	if(*reg == 'x'){ //64-bit width
		return (imm / 8);
	}else if(*reg == 'w'){
		return (imm / 4);
	}
	
	fprintf(stderr, "Unknown register width");
	return 0;
}

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

	bool loadLiteral = (numparams == 3) & (*params[2] != '[');
	//load literal is ldr with 2 args, sdts have an extra argument
	//unsigned offset can also just have 3 params, the 3rd being a [regname]
	
	if(loadLiteral){
		printf("debug: this is a load literal\n");
		toReturn = 0x18000000;

		char *value = params[2]; //#imm or label offset
		uint32_t simm19;
		if(is_imm(value)){
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

	//common algorithms for non-load literal sdt instrs
	if(!loadLiteral){
		char *xn_name = removeBrackets(params[2]);
		printf("debug: xn_name = %s\n", xn_name);
		int amode = mode(params, numparams);
		printf("debug: addressing mode = %d\n", amode);
		uint8_t xn = obtain_reg_num(xn_name);
		toReturn |= (xn << 5);
		switch(amode){
			case(MODE_UNSIGNED_OFFSET): toReturn |= 0x01000000;
						    //set U bit
						    int imm12;
						    if(numparams < 4){
						    	imm12 = 0;
						    }else{
						    	imm12 = getImm12(params[3], target);
						    }
						    imm12 &= 0xfff; //make sure it's 12 bits
						    toReturn |= (imm12 << 10);
						    break;

			case(MODE_PRE_INDEX): //
					      break;

			case(MODE_POST_INDEX): //
					       break;

			case(MODE_REG_OFFSET): //
					      break;

			default: fprintf(stderr, "Unknown addressing mode");
				 return 0;
				 break;
		}
	}

	update_sf(&toReturn, 30, target); //update register width based on target register
	toReturn |= rt; //replace last 5 bits with target reg number
	printf("debug: toReturn = %x\n", toReturn);

	return toReturn; 
}
