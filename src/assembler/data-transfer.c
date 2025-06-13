#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "symtable.h"
#include "data-transfer.h"
#include "assembly-utils.h"
#include "constants.h"
#include "instr-formats.h"

#define MAX_PARAMS 5
#define MIN_PARAMS 3

static char *removeBrackets(char *str){
	char *without = strtok(str, "[ ]");
	return without;
}

//returns addressing mode or 0 if error occurs
static int mode(char **params, int numparams){
	if(numparams < 3){
		fprintf(stderr, "Unknown addressing mode");
		return 0;
	}
	if(numparams == 3){ return MODE_UNSIGNED_OFFSET; }
	//3 params -> zero unsigned offset
	int len = strlen(params[3]);
	char last_char = params[3][len-1];
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
	imm_str = strchr(imm_str, '#');
	if (!imm_str) { return 0; }
	while (*imm_str && isspace(*imm_str)) { imm_str++; }

	int imm = extract_imm(imm_str);
	if(*reg == 'x'){ //64-bit width
		return (imm / WORD_SIZE_64);
	}else if(*reg == 'w'){
		return (imm / WORD_SIZE_32);
	}
	
	fprintf(stderr, "Unknown register width");
	return 0;
}

static int getSimm9(char *imm){
        while(*imm && !isdigit(*imm) && *imm != '-') { imm++; }	
	return extract_imm(imm);
}

// Return 0 if success, 1 if fail
int dt(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a data transfer instruction\n");

	if(numparams > MAX_PARAMS || numparams < MIN_PARAMS){
		printf("debug: incorrect no. params\n");
		fprintf(stderr, "Unexpected no. parameters for dt instr\n");
		return EXIT_FAILURE;
	}
	char *type = params[0]; // load/store instruction
	char *target = params[1]; // first argument is target register
	uint8_t rt = obtain_reg_num(target);

	bool loadLiteral = (numparams == 3) && (*params[2] != '[');
	//load literal is ldr with 2 args, sdts have an extra argument
	//unsigned offset can also just have 3 params, the 3rd being a [regname]	
	if(loadLiteral){
		printf("debug: this is a load literal\n");
		// Set the instruction base 
		*toReturn = 0x18000000;
		uint32_t simm19; 

		char *value = params[2]; //#imm or label offset 
		if(is_imm(value)){
			//immediate valuae
			simm19 = extract_imm(value);
		}else{
			//value is a label offset (label addr - curr addr)
			simm19 = strtol(value, NULL, 10) / WORD_SIZE_32;
		}
		*toReturn |= mask_shift_val(simm19, sdt_format.simm19.bits, sdt_format.simm19.index); //set bits 5-23 with simm19 value
	}else if(!strcmp(type, "ldr")){
		//load instruction, no load literal
		*toReturn = 0xb8400000; //L bit set
	}else if(!strcmp(type, "str")){
		//store instruction
		*toReturn = 0xb8000000; //L bit not set
	}else{
		fprintf(stderr, "Data transfer instruction is not ldr or str\n");
		return EXIT_FAILURE;
	}

	//common algorithms for non-load literal sdt instrs
	if(!loadLiteral){
		char *xn_name = removeBrackets(params[2]);
		int amode = mode(params, numparams);
		uint8_t xn = obtain_reg_num(xn_name);
		*toReturn |= mask_shift_val(xn, sdt_format.xn.bits, sdt_format.xn.index);
		uint32_t simm9; 
		switch(amode){
			case(MODE_UNSIGNED_OFFSET):
				printf("debug: unsigned offset\n"); 
				*toReturn |= mask_shift_val(1, sdt_format.U.bits, sdt_format.U.index);
				//set U bit
				int imm12;
				if(numparams < 4){
				   	imm12 = 0;
				}else{
				   	imm12 = getImm12(params[3], target);
				}
				//imm12 &= 0xfff; //make sure it's 12 bits
				*toReturn |= mask_shift_val(imm12, sdt_format.offset.bits, sdt_format.offset.index);
				break;

			case(MODE_PRE_INDEX): 
				printf("debug: pre-index\n"); 
				*toReturn |= (1 << 10); 
				*toReturn |= mask_shift_val(1, sdt_format.I.bits, sdt_format.I.index); //set I bit
				simm9 = getSimm9(params[3]);
				//simm9 &= 0x1ff; //make sure it's 9 bits
				*toReturn |= mask_shift_val(simm9, sdt_format.simm9.bits, sdt_format.simm9.index);
				break;

			case(MODE_POST_INDEX): 
				printf("debug: post-index\n"); 
				*toReturn |= (1 << 10); //set bit indicating post index
				simm9 = getSimm9(params[3]);
				//simm9 &= 0x1ff; 
				*toReturn |= mask_shift_val(simm9, sdt_format.simm9.bits, sdt_format.simm9.index);
				break;

			case(MODE_REG_OFFSET): 
				printf("debug: reg offset\n"); 
				*toReturn |= 0x00206800; //update the instruction base
				uint8_t xm = obtain_reg_num(removeBrackets(params[3]));
				*toReturn |= mask_shift_val(xm, sdt_format.xm.bits, sdt_format.xm.index);
				break;

			default: 
				fprintf(stderr, "Unknown addressing mode");
				return EXIT_FAILURE;
				break;
		}
	}

	update_sf(toReturn, sdt_format.sf.index, target); //update register width based on target register
	*toReturn |= rt; //replace last 5 bits with target reg number
	printf("debug: toReturn = %x\n", *toReturn);

	return EXIT_SUCCESS; 
}
