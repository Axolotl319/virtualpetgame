#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "data-processing.h"
#include "assembly-utils.h"
//#include "tokenise_params.h"

static bool is_imm(char *param) {
	return *param == '#';
}

// Returns the encoded instruction if success, -1 if fail
int arith(char **params, int numparams) {
	printf("debug: this is an arithmetic expression\n"); 
	uint32_t toReturn = 0; 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 

	toReturn |= rd; 
	toReturn |= (rn << 5); 

	update_sf(&toReturn, 31, params[1]); 

	// Update opc
	if (!strcmp(params[0], "adds") || !strcmp(params[0], "subs")) {
		toReturn |= (1 << 29); 
	}
	if (!strcmp(params[0], "sub") || !strcmp(params[0], "subs")) {
		toReturn |= (1 << 30); 
	}

	if (is_imm(params[3])) {
		// Then it is immediate value arithmetic
		toReturn |= (1 << 28); 

		// Update opi
		toReturn |= (1 << 24);
		
		// Obtain imm12 value 
		toReturn |= (extract_imm(params[3]) << 10);

		// Update shift bit if needed
		if (numparams == 5 && obtain_shift_amt(params[4]) == 12) {
		   toReturn |= (1 << 22);
		}
	} else {
		// Else it is register arithmetic 
		toReturn |= (1 << 27); 
		toReturn |= (1 << 25); 
		
		uint8_t rm = obtain_reg_num(params[3]); 
		toReturn |= (rm << 16); 

		// Update opr
		toReturn |= (1 << 24); 

		// Update shift and operand if needed
		if (numparams == 5) {
		   char shifttype[4];
		   sscanf(params[4], "%s", shifttype); 
		   uint8_t oper = obtain_shift_amt(params[4]);
		   if (!strcmp(shifttype, "lsr")) {
		   	toReturn |= (1 << 22); 
		   } else if (!strcmp(shifttype, "asr")) {
		   	toReturn |= (1 << 23); 
		   }
		   toReturn |= (oper << 10); 
		}
	}
	printf("debug: arithmetic looks like this: %x\n", toReturn); 
	return toReturn; 
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
	toReturn |= (extract_imm(params[2]) << 5);

	// If a left shift exists, update the instruction 
	if (numparams == 4) {
		toReturn |= ((obtain_shift_amt(params[3]) / 16) << 21);
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
