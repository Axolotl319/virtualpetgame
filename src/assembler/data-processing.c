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

static void set_shift_info(uint32_t *toReturn, char *param) {
	char shifttype[4];
	sscanf(param, "%s", shifttype); 
	uint8_t oper = obtain_shift_amt(param);
	if (!strcmp(shifttype, "lsr")) {
		*toReturn |= (1 << 22); 
	} else if (!strcmp(shifttype, "asr")) {
	   	*toReturn |= (1 << 23); 
	}
	*toReturn |= (oper << 10); 
}

// Returns the encoded instruction if success, -1 if fail
int arith(char **params, int numparams) {
	printf("debug: this is an arithmetic expression\n"); 
	uint32_t toReturn = 0; 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 

	toReturn |= rd; 
	toReturn |= (rn << 5); 

	update_sf(&toReturn, 31, params[2]); 

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
			set_shift_info(&toReturn, params[4]); 	   
		}
	}
	printf("debug: Resulting arithmetic output: %x\n", toReturn); 
	return toReturn; 
}

int logic(char **params, int numparams) {
	printf("debug: this is a logic expression\n"); 
	uint32_t toReturn = 0x0a000000;

	update_sf(&toReturn, 31, params[1]); 

	// Obtain and update all the registers 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 
	uint8_t rm = obtain_reg_num(params[3]);
        toReturn |= rd; 
	toReturn |= (rn << 5); 
	toReturn |= (rm << 16); 

	// Set opc and N depending on the mnemonic 
	uint8_t opc = 0; 
	uint8_t n = 0; 
	if (!strcmp(params[0], "bic")) {
		n = 1; 
	} else if (!strcmp(params[0], "orr")) {
		opc = 1; 
	} else if (!strcmp(params[0], "orn")) {
		opc = 1; 
		n = 1; 
	} else if (!strcmp(params[0], "eor")) {
		opc = 2; 
	} else if (!strcmp(params[0], "eon")) {
		opc = 2; 
		n = 1; 
	} else if (!strcmp(params[0], "ands")) {
		opc = 3; 
	} else if (!strcmp(params[0], "bics")) {
		opc = 3; 
		n = 1; 
	}
	toReturn |= (opc << 29); 
	toReturn |= (n << 21); 

	// Set the shift type if needed
	if (numparams == 5) {
		set_shift_info(&toReturn, params[4]);	
	}
	
	printf("debug: Resulting logic output: %x\n", toReturn); 
	return toReturn; 
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

	printf("debug: Resulting wide move output: %x\n", toReturn); 
	return toReturn; 
}

#define insert_zero(numparams) params[numparams] = params[numparams-1]; params[numparams-1] = "xzr";

int single_op_dest(char **params, int numparams) {
	printf("debug: this is a single op and destination expression\n"); 
	if (!strcmp(params[0], "mul")) {
		params[0] = "madd"; 
		params[numparams++] = "xzr"; 
		return multiply(params, numparams); 
	} else if (!strcmp(params[0], "mneg")) {
		params[0] = "msub"; 
		params[numparams++] = "xzr"; 
		return multiply(params, numparams); 
	} else if (!strcmp(params[0], "mov")) {
		params[0] = "orr"; 
		params[numparams++] = "xzr"; 
		return logic(params, numparams); 
	} else if (!strcmp(params[0], "mvn")) {
		params[0] = "orn"; 
		insert_zero(numparams); 
		return logic(params, numparams++); 
	} else if (!strcmp(params[0], "neg")) {
		params[0] = "sub"; 
		insert_zero(numparams);  
		return arith(params, numparams++); 
	} 
	params[0] = "subs"; 
	insert_zero(numparams); 
	return arith(params, numparams++); 
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
	char *zero = strchr(params[1], 'x') ? "xzr" : "wzr";
	
	// Insert the zero register at params[1]
	for (int i=numparams; i>1; i--) {
	   params[i] = params[i-1];  
	}
	params[1] = zero; 
	numparams++; 

	uint32_t toReturn = 0x1f; 

	if (!strcmp(params[0], "tst")) {
	   params[0] = "ands"; 
	   toReturn |= logic(params, numparams); 
	} else if (!strcmp(params[0], "cmp")) {
	   params[0] = "subs";  
	   toReturn |= arith(params, numparams);
	} else {
	   params[0] = "adds"; 
	   toReturn |= arith(params, numparams);
	} 
	return toReturn; 
}
