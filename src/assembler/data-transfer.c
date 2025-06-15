#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "symtable.h"
#include "data-transfer.h"
#include "assembly-utils.h"
#include "constants.h"
#include "instr-formats.h"

#define MAX_PARAMS 5
#define MIN_PARAMS 3

#define LOADLIT_BASE 0x18000000
#define LDR_BASE 0xb8400000
#define STR_BASE 0xb8000000
#define REG_OFFSET_BASE 0x00206800 
#define PRE_POST_BASE 0x00000400

static char *removeBrackets(char *str){
	char *without = strtok(str, "[ ]");
	return without;
}

//returns addressing mode or 0 if error occurs
static int mode(char **params, int numparams){
	if(numparams < MIN_PARAMS){
		fprintf(stderr, "Unknown addressing mode");
		return 0;
	}
	assert(numparams >= MIN_PARAMS);

	if(numparams == MIN_PARAMS){ return MODE_UNSIGNED_OFFSET; }
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

//puts imm12 into int pointer
static int getImm12(char *imm_str, char *reg, int *imm12){
	imm_str = strchr(imm_str, '#');
	if (!imm_str) { return EXIT_FAILURE; }
	assert(imm_str != NULL);
	while (*imm_str && isspace(*imm_str)) { imm_str++; }

	int imm = extract_imm(imm_str);
	if(*reg == 'x'){ //64-bit width
		*imm12 = (imm / WORD_SIZE_64);
	}else if(*reg == 'w'){
		*imm12 = (imm / WORD_SIZE_32);
	}else{
		fprintf(stderr, "Unknown register width");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

static int getSimm9(char *imm){
        while(*imm && !isdigit(*imm) && *imm != '-') { imm++; }	
	return extract_imm(imm);
}

// Return 0 if success, 1 if fail
int dt(char **params, int numparams, uint32_t *instr) {
	//Bounds check the number of parameters
	if(numparams > MAX_PARAMS || numparams < MIN_PARAMS){
		fprintf(stderr, "Unexpected no. parameters for dt instr\n");
		return EXIT_FAILURE;
	}
	assert(numparams <= MAX_PARAMS && numparams >= MIN_PARAMS);

	char *type = params[0]; // load/store instruction
	char *target = params[1]; // first argument is target register
	uint8_t rt = obtain_reg_num(target);

	bool loadLiteral = (numparams == MIN_PARAMS) && (*params[2] != '[');
	//load literal is ldr with 2 args, sdts have an extra argument
	//unsigned offset can also just have 3 params, the 3rd being a [regname]	
	if(loadLiteral){
		// Set the instruction base 
		*instr = LOADLIT_BASE;
		uint32_t simm19; 

		char *value = params[2]; //#imm or label offset 
		if(is_imm(value)){
			//immediate valuae
			simm19 = extract_imm(value);
		}else{
			//value is a label offset (label addr - curr addr)
			simm19 = strtol(value, NULL, 10) / WORD_SIZE_32;
		}
		//set bits 5-23 with simm19 value
		*instr |= place_bits(simm19, sdt_format.simm19.bits, sdt_format.simm19.index); 

	}else if(!strcmp(type, "ldr")){
		//load instruction, no load literal
		*instr = LDR_BASE; //L bit set
	}else if(!strcmp(type, "str")){
		//store instruction
		*instr = STR_BASE; //L bit not set
	}else{
		fprintf(stderr, "Data transfer instruction is not ldr or str\n");
		return EXIT_FAILURE;
	}

	//common algorithms for non-load literal sdt instrs
	if(!loadLiteral){
		char *xn_name = removeBrackets(params[2]);
		int amode = mode(params, numparams);
		uint8_t xn = obtain_reg_num(xn_name);
		*instr |= place_bits(xn, sdt_format.xn.bits, sdt_format.xn.index);
		uint32_t simm9; 
		switch(amode){
			case(MODE_UNSIGNED_OFFSET):
				*instr |= place_bits(1, sdt_format.U.bits, sdt_format.U.index);
				//set U bit
				int imm12;
				if(numparams == MIN_PARAMS){
				   	imm12 = 0;
				}else{
				   	if (getImm12(params[3], target, &imm12)) { return EXIT_FAILURE; }
				}
				assert(&imm12 != NULL);
				*instr |= place_bits(imm12, sdt_format.offset.bits, sdt_format.offset.index);
				break;

			case(MODE_PRE_INDEX): 
				*instr |= PRE_POST_BASE; 
				*instr |= place_bits(1, sdt_format.I.bits, sdt_format.I.index); //set I bit
				simm9 = getSimm9(params[3]);
				*instr |= place_bits(simm9, sdt_format.simm9.bits, sdt_format.simm9.index);
				break;

			case(MODE_POST_INDEX): 
				*instr |= PRE_POST_BASE; //set bit indicating post index
				simm9 = getSimm9(params[3]);
				*instr |= place_bits(simm9, sdt_format.simm9.bits, sdt_format.simm9.index);
				break;

			case(MODE_REG_OFFSET): 
				*instr |= REG_OFFSET_BASE; //update the instruction base
				uint8_t xm = obtain_reg_num(removeBrackets(params[3]));
				*instr |= place_bits(xm, sdt_format.xm.bits, sdt_format.xm.index);
				break;

			default: 
				fprintf(stderr, "Unknown addressing mode");
				return EXIT_FAILURE;
				break;
		}
	}

	update_sf(instr, sdt_format.sf.index, target); //update register width based on target register
	*instr |= place_bits(rt, sdt_format.rt.bits, sdt_format.rt.index); //replace last 5 bits with target reg number
	
	return EXIT_SUCCESS; 
}
