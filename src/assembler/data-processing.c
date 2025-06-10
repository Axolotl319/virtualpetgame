#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "data-processing.h"
#include "assembly-utils.h"
//#include "tokenise_params.h"

// Returns the encoded instruction if success, -1 if fail
int arith(char **params, int numparams) {
	printf("debug: this is an arithmetic expression\n"); 
	return 1; 
}

int logic(char **params, int numparams) {
	printf("debug: this is a logic expression\n"); 
	return 1; 
}

int wmove(char **params, int numparams) {
	printf("debug: this is a wide move expression\n");
        uint32_t toReturn = 0x12800000;

	update_sf(&toReturn, 31, params[1]); 

	// Update rd 
	toReturn |= obtain_reg_num(params[1]); 

	// Update opc
	if (!strcmp(params[0], "movz")) {
		toReturn |= (1 << 30); 
	} else if (!strcmp(params[0], "movk")) {
		toReturn |= (1 << 30); 
		toReturn |= (1 << 29); 
	}

	// Extract imm16 and update toReturn 
	if (strchr(params[2], 'x') != NULL) {
		toReturn |= (extract_imm(params[2]) << 5);
	} else {
		toReturn |= (extract_imm_dec(params[2]) << 5);
	}

	// If a left shift exists, update the instruction 
	if (numparams == 4) {
		char shifttype[5]; 
		char shiftamt[5]; 
		sscanf(params[3], "%s %s", shifttype, shiftamt);	
		toReturn |= ((extract_imm_dec(shiftamt) / 16) << 21);
	}

	printf("debug: toReturn looks like this: %x\n", toReturn); 
	return toReturn; 
}

int single_op_dest(char **params, int numparams) {
	printf("debug: this is a single op and destination expression\n"); 
	return 1; 
}

int multiply(char **params, int numparams) {
	printf("debug: this is a multiply expression\n");
        assert(numparams == 5); 	
	uint32_t toReturn = 0x1b000000; 
 
	update_sf(&toReturn, 31, params[1]); 

	// Update x: 
	if (strcmp(params[0], "msub") == 0) {
		toReturn |= (1 << 15); 
	}

	// Obtain the register numbers: 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 
	uint8_t rm = obtain_reg_num(params[3]); 
	uint8_t ra = obtain_reg_num(params[4]);

	toReturn |= rd; 
	toReturn |= (rn << 5); 
	toReturn |= (ra << 10); 
	toReturn |= (rm << 16); 	
	 
	printf("Resulting multiply output: %x\n", toReturn); 
	return toReturn; 
}

int compare(char **params, int numparams) {
	printf("debug: this is a compare/test expression\n"); 
	return 1; 
}
