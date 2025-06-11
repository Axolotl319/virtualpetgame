#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "data-processing.h"
#include "assembly-utils.h"
#include "instr-formats.h"

// Sets the bits for shift type and shift amount in toReturn. 
// Returns 0 if failure, returns 1 if success. 
static int set_shift_info(uint32_t *toReturn, bool logic, char *param) {
	char shifttype[4];
	if (sscanf(param, "%s", shifttype) == EOF) {
		fprintf(stderr, "Shift type could not be read.\n"); 
		return 0; 
	}	
	uint8_t oper = obtain_shift_amt(param);
	if (!strcmp(shifttype, "lsr")) {
		*toReturn |= (1 << regdp_format.shift.index); 
	} else if (!strcmp(shifttype, "asr")) {
	   	*toReturn |= (2 << regdp_format.shift.index); 
	} else if (logic && !strcmp(shifttype, "ror")) {
		*toReturn |= (3 << regdp_format.shift.index); 
	} else if (!strcmp(shifttype, "lsl")) {
		/* EMPTY BODY */
	} else {
		fprintf(stderr, "Shift type not recognized.\n"); 
		return 0; 
	}
	*toReturn |= (oper << regdp_format.operand.index);
        return 1; 	
}

// Parses arithmetic instructions into decimal format 
int arith(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is an arithmetic expression\n");
        if (numparams != 4 && numparams != 5) {
		fprintf(stderr, "Incorrect number of parameters.\n"); 
		return EXIT_FAILURE; 
	}	
	*toReturn = 0; 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 

	*toReturn |= rd; 
	*toReturn |= (rn << immdp_format.rn.index); 

	update_sf(toReturn, immdp_format.sf.index, params[2]); 

	// Update opc
	if (!strcmp(params[0], "adds") || !strcmp(params[0], "subs")) {
		*toReturn |= (1 << immdp_format.opc.index); 
	}
	if (!strcmp(params[0], "sub") || !strcmp(params[0], "subs")) {
		*toReturn |= (2 << immdp_format.opc.index); 
	}

	if (is_imm(params[3])) {
		// Then it is immediate value arithmetic
		// Set the constant base at bit 28
		*toReturn |= (1 << 28); 

		// Update opi (binary 010) 
		*toReturn |= (2 << immdp_format.opi.index);
		
		// Obtain imm12 value 
		*toReturn |= (extract_imm(params[3]) << immdp_format.imm12.index);

		// Update shift bit if needed
		if (numparams == 5 && obtain_shift_amt(params[4]) == 12) {
		   *toReturn |= (1 << immdp_format.sh.index);
		}
	} else {
		// Else it is register arithmetic
		// Set constant bases at bits 27 and 25 
		*toReturn |= (1 << 27); 
		*toReturn |= (1 << 25); 
		
		uint8_t rm = obtain_reg_num(params[3]); 
		*toReturn |= (rm << regdp_format.rm.index); 

		// Update opr (binary 1000)
		*toReturn |= (8 << regdp_format.opr.index); 

		// Update shift and operand if needed
		if (numparams == 5) {
			if (!set_shift_info(toReturn, false, params[4])) {
				return EXIT_FAILURE; 
			}		
		}
	} 
	return EXIT_SUCCESS; 
}

int logic(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a logic expression\n"); 
	if (numparams != 4 && numparams != 5) {
		fprintf(stderr, "Invalid number of parameters.\n");
	        return EXIT_FAILURE; 	
	}
	// Set up the instruction base 
	*toReturn = 0x0a000000;

	update_sf(toReturn, regdp_format.sf.index, params[1]); 

	// Obtain and update all the registers 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 
	uint8_t rm = obtain_reg_num(params[3]);
        *toReturn |= rd; 
	*toReturn |= (rn << regdp_format.rn.index); 
	*toReturn |= (rm << regdp_format.rm.index); 

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
	*toReturn |= (opc << regdp_format.opc.index); 
	*toReturn |= (n << regdp_format.N.index); 

	// Set the shift type if needed
	if (numparams == 5) {
		if (!set_shift_info(toReturn, true, params[4])) {
			return EXIT_FAILURE; 
		}	
	}
	 
	return EXIT_SUCCESS; 
}

int wmove(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a wide move expression\n");
        if (numparams != 3 && numparams != 4) {
		fprintf(stderr, "Invalid number of parameters.\n");
	        return EXIT_FAILURE; 	
	}
	*toReturn = 0x12800000;

	update_sf(toReturn, immdp_format.sf.index, params[1]); 

	// Update rd 
	*toReturn |= obtain_reg_num(params[1]); 

	// Update opc
	if (!strcmp(params[0], "movz")) {
		*toReturn |= (2 << immdp_format.opc.index); 
	} else if (!strcmp(params[0], "movk")) {
		*toReturn |= (3 << immdp_format.opc.index);  
	} else if (!strcmp(params[0], "movn")) {
		/* EMPTY BODY */
	} else {
		fprintf(stderr, "Unrecognized wide move mnemonic.\n"); 
		return EXIT_FAILURE; 
	}

	// Extract imm16 and update toReturn 
	*toReturn |= (extract_imm(params[2]) << immdp_format.imm16.index);

	// If a left shift exists, update the instruction 
	if (numparams == 4) {
		uint8_t bit = obtain_shift_amt(params[3]) / 16; 
		*toReturn |= (bit << immdp_format.hw.index);
	}
 
	return EXIT_SUCCESS; 
}

#define insert_zero(numparams) params[numparams] = params[numparams-1]; params[numparams-1] = "xzr";

int single_op_dest(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a single op and destination expression\n"); 
	if (numparams != 3 && numparams != 4) {
		fprintf(stderr, "Invalid number of parameters.\n"); 
		return EXIT_FAILURE; 
	}
	if (!strcmp(params[0], "mul")) {
		params[0] = "madd"; 
		params[numparams++] = "xzr"; 
		multiply(params, numparams, toReturn); 
	} else if (!strcmp(params[0], "mneg")) {
		params[0] = "msub"; 
		params[numparams++] = "xzr"; 
		multiply(params, numparams, toReturn); 
	} else if (!strcmp(params[0], "mov")) {
		params[0] = "orr"; 
		params[numparams++] = "xzr"; 
		logic(params, numparams, toReturn); 
	} else if (!strcmp(params[0], "mvn")) {
		params[0] = "orn"; 
		insert_zero(numparams); 
		logic(params, numparams++, toReturn); 
	} else if (!strcmp(params[0], "neg")) {
		params[0] = "sub"; 
		insert_zero(numparams);  
		arith(params, numparams++, toReturn); 
	} else if (!strcmp(params[0], "negs")) {
		params[0] = "subs"; 
		insert_zero(numparams); 
		arith(params, numparams++, toReturn);
	} else {
		fprintf(stderr, "Unrecognized mnemonic.\n"); 
		return EXIT_FAILURE; 
	}
	return EXIT_SUCCESS; 
}

int multiply(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a multiply expression\n");
        if (numparams != 5) {
		fprintf(stderr, "Invalid number of parameters.\n"); 
		return EXIT_FAILURE; 
	}
	*toReturn = 0x1b000000; 
 
	update_sf(toReturn, regdp_format.sf.index, params[1]); 

	// Update x: 
	if (!strcmp(params[0], "msub")) {
		*toReturn |= (1 << regdp_format.x.index); 
	} else if (!strcmp(params[0], "madd")) {
		/* EMPTY BODY */
	} else {
		fprintf(stderr, "Unrecognized multiply mnemonic.\n"); 
		return EXIT_FAILURE; 
	}

	// Obtain the register numbers: 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 
	uint8_t rm = obtain_reg_num(params[3]); 
	uint8_t ra = obtain_reg_num(params[4]);

	*toReturn |= rd; 
	*toReturn |= (rn << regdp_format.rn.index); 
	*toReturn |= (ra << regdp_format.ra.index); 
	*toReturn |= (rm << regdp_format.rm.index); 	
	 
	return EXIT_SUCCESS; 
}

int compare(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a compare/test expression\n"); 
	char *zero = strchr(params[1], 'x') ? "xzr" : "wzr";
	
	// Insert the zero register at params[1]
	for (int i=numparams; i>1; i--) {
	   params[i] = params[i-1];  
	}
	params[1] = zero; 
	numparams++; 

	if (!strcmp(params[0], "tst")) {
	   params[0] = "ands"; 
	   logic(params, numparams, toReturn);
	} else if (!strcmp(params[0], "cmp")) {
	   params[0] = "subs";
	   arith(params, numparams,  toReturn);  
	} else if (!strcmp(params[0], "cmn")) {
	   params[0] = "adds";
	   arith(params, numparams, toReturn); 
	} else {
		fprintf(stderr, "Unrecognized compare mnemonic.\n"); 
		return EXIT_FAILURE; 
	}
	*toReturn |= 0x1f; 
	return EXIT_SUCCESS; 
}
