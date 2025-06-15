#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "data-processing.h"
#include "assembly-utils.h"
#include "instr-formats.h"
#include "constants.h"

//base instructions
#define IMM_BASE 0x10000000
#define REG_BASE 0x0A000000
#define LOGIC_BASE 0x0A000000
#define MOV_BASE 0x12800000
#define MUL_BASE 0x1b000000
#define ARITH_OPR_BASE 0x8
#define OPI_ARITH 0x2

//param numbers for arith/logic instrs
#define MIN_AL_PARAMS 4
#define MAX_AL_PARAMS 5
//param numbers for move instrs
#define MIN_MOV_PARAMS 3
#define MAX_MOV_PARAMS 4
//param numbers for single operand dest instrs
#define MIN_SOP_PARAMS 3
#define MAX_SOP_PARAMS 4
//param number for multiply instrs
#define MUL_PARAMS 5

#define IMM12_LEN 12
#define IMM16_LEN 16

// The following macro inserts the zero register in place 
// of rm
#define insert_zero_reg(numparams) params[numparams] = params[numparams-1]; params[numparams-1] = "xzr";


// Sets the bits for shift type and shift amount in toReturn. 
// Returns true for failure and false for success
static int set_shift_info(uint32_t *toReturn, bool logic, char *param) {
	char shifttype[4];
	if (sscanf(param, "%s", shifttype) == EOF) {
		fprintf(stderr, "Shift type could not be read.\n"); 
		return EXIT_FAILURE; 
	}	
	uint8_t oper = obtain_shift_amt(param);

	int shift = LSL;
	if (!strcmp(shifttype, "lsr")) {
		shift = LSR; 
	} else if (!strcmp(shifttype, "asr")) {
	   	shift = ASR; 
	} else if (logic && !strcmp(shifttype, "ror")) {
		shift = ROR; 
	} else if (!strcmp(shifttype, "lsl")) {
		/* EMPTY BODY */
	} else {
		fprintf(stderr, "Shift type not recognized.\n"); 
		return EXIT_FAILURE; 
	}
	*toReturn |= place_bits(shift, regdp_format.shift.bits, regdp_format.shift.index);
	*toReturn |= place_bits(oper, regdp_format.operand.bits, regdp_format.operand.index);
        return EXIT_SUCCESS; 	
}

// Parses arithmetic instructions into decimal format 
int arith(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is an arithmetic expression\n");
        if (numparams != MIN_AL_PARAMS && numparams != MAX_AL_PARAMS) {
		fprintf(stderr, "Incorrect number of parameters.\n"); 
		return EXIT_FAILURE; 
	}	
	*toReturn = 0; 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 

	*toReturn |= place_bits(rd, immdp_format.rd.bits, immdp_format.rd.index); 
	*toReturn |= place_bits(rn, immdp_format.rn.bits, immdp_format.rn.index); 

	update_sf(toReturn, immdp_format.sf.index, params[2]); 

	// Update opc
	int opc = ARITH_ADD;

	if (!strcmp(params[0], "adds")) {
		opc = ARITH_ADDS;
	} else if (!strcmp(params[0], "sub")) {
		opc = ARITH_SUB;
	} else if (!strcmp(params[0], "subs")) {
		opc = ARITH_SUBS;
	}

	*toReturn |= place_bits(opc, immdp_format.opc.bits, immdp_format.opc.index);

	if (is_imm(params[3])) {
		// Then it is immediate value arithmetic
		// Set the constant base at bit 28 
		*toReturn |= IMM_BASE;

		// Update opi (binary 010) 
		*toReturn |= place_bits(OPI_ARITH, immdp_format.opi.bits, immdp_format.opi.index);
		
		// Obtain imm12 value 
		*toReturn |= place_bits(extract_imm(params[3]), immdp_format.imm12.bits, immdp_format.imm12.index);

		// Update shift bit if needed
		if (numparams == MAX_AL_PARAMS && obtain_shift_amt(params[4]) == IMM12_LEN) {
		   *toReturn |= place_bits(1, immdp_format.sh.bits, immdp_format.sh.index);
		}
	} else {
		// Else it is register arithmetic
		// Set constant bases at bits 27 and 25 
		*toReturn |= REG_BASE;
		
		uint8_t rm = obtain_reg_num(params[3]); 
		*toReturn |= place_bits(rm, regdp_format.rm.bits, regdp_format.rm.index); 

		// Update opr (binary 1000)
		*toReturn |= place_bits(ARITH_OPR_BASE, regdp_format.opr.bits, regdp_format.opr.index); 

		// Update shift and operand if needed
		if (numparams == MAX_AL_PARAMS) {
			if (set_shift_info(toReturn, false, params[4])) {
				return EXIT_FAILURE; 
			}		
		}
	} 
	return EXIT_SUCCESS; 
}

int logic(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a logic expression\n"); 
	if (numparams != MIN_AL_PARAMS && numparams != MAX_AL_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n");
	        return EXIT_FAILURE; 	
	}
	// Set up the instruction base 
	*toReturn = LOGIC_BASE;

	update_sf(toReturn, regdp_format.sf.index, params[1]); 

	// Obtain and update all the registers 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 
	uint8_t rm = obtain_reg_num(params[3]);
        *toReturn |= place_bits(rd, regdp_format.rd.bits, regdp_format.rd.index); 
	*toReturn |= place_bits(rn, regdp_format.rd.bits, regdp_format.rn.index); 
	*toReturn |= place_bits(rm, regdp_format.rm.bits, regdp_format.rm.index); 

	// Set opc and N (negate flag) depending on the mnemonic 
	uint8_t opc = 0; 
	uint8_t n = 0; 
	if (!strcmp(params[0], "bic")) {
		n = 1; 
	} else if (!strcmp(params[0], "orr")) {
		opc = LOG_OR; 
	} else if (!strcmp(params[0], "orn")) {
		opc = LOG_OR; 
		n = 1; 
	} else if (!strcmp(params[0], "eor")) {
		opc = LOG_XOR; 
	} else if (!strcmp(params[0], "eon")) {
		opc = LOG_XOR; 
		n = 1; 
	} else if (!strcmp(params[0], "ands")) {
		opc = LOG_ANDS; 
	} else if (!strcmp(params[0], "bics")) {
		opc = LOG_ANDS; 
		n = 1; 
	}
	*toReturn |= place_bits(opc, regdp_format.opc.bits, regdp_format.opc.index); 
	*toReturn |= place_bits(n, regdp_format.N.bits, regdp_format.N.index); 

	// Set the shift type if needed
	if (numparams == MAX_AL_PARAMS) {
		if (set_shift_info(toReturn, true, params[4])) {
			return EXIT_FAILURE; 
		}	
	}
	 
	return EXIT_SUCCESS; 
}

int wmove(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a wide move expression\n");
        if (numparams != MIN_MOV_PARAMS && numparams != MAX_MOV_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n");
	        return EXIT_FAILURE; 	
	}
	// Set the instruction base 
	*toReturn = MOV_BASE;

	update_sf(toReturn, immdp_format.sf.index, params[1]); 

	// Update rd 
	*toReturn |= place_bits(obtain_reg_num(params[1]), immdp_format.rd.bits, immdp_format.rd.index); 

	// Update opc
	if (!strcmp(params[0], "movz")) {
		*toReturn |= place_bits(MOVZ, immdp_format.opc.bits, immdp_format.opc.index); 
	} else if (!strcmp(params[0], "movk")) {
		*toReturn |= place_bits(MOVK, immdp_format.opc.bits, immdp_format.opc.index);  
	} else if (!strcmp(params[0], "movn")) {
		/* EMPTY BODY */
	} else {
		fprintf(stderr, "Unrecognized wide move mnemonic.\n"); 
		return EXIT_FAILURE; 
	}

	// Extract imm16 and update toReturn 
	*toReturn |= place_bits(extract_imm(params[2]), immdp_format.imm16.bits, immdp_format.imm16.index);

	// If a left shift exists, update the instruction 
	if (numparams == MAX_MOV_PARAMS) {
		uint8_t bit = obtain_shift_amt(params[3]) / IMM16_LEN; 
		*toReturn |= place_bits(bit, immdp_format.hw.bits, immdp_format.hw.index);
	}
 
	return EXIT_SUCCESS; 
}


int single_op_dest(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a single op and destination expression\n"); 
	if (numparams != MIN_SOP_PARAMS && numparams != MAX_SOP_PARAMS) {
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
		insert_zero_reg(numparams);   
		logic(params, ++numparams, toReturn); 
	} else if (!strcmp(params[0], "mvn")) {
		params[0] = "orn"; 
		insert_zero_reg(numparams); 
		logic(params, ++numparams, toReturn); 
	} else if (!strcmp(params[0], "neg")) {
		params[0] = "sub"; 
		insert_zero_reg(numparams);  
		arith(params, ++numparams, toReturn); 
	} else if (!strcmp(params[0], "negs")) {
		params[0] = "subs"; 
		insert_zero_reg(numparams); 
		arith(params, ++numparams, toReturn);
	} else {
		fprintf(stderr, "Unrecognized mnemonic.\n"); 
		return EXIT_FAILURE; 
	}
	return EXIT_SUCCESS; 
}

int multiply(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a multiply expression\n");
        if (numparams != MUL_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n"); 
		return EXIT_FAILURE; 
	}
	// Set the instruction base 
	*toReturn = MUL_BASE; 
 
	update_sf(toReturn, regdp_format.sf.index, params[1]); 

	// Update x: 
	if (!strcmp(params[0], "msub")) {
		*toReturn |= place_bits(1, regdp_format.x.bits, regdp_format.x.index); 
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

	*toReturn |= place_bits(rd, regdp_format.rd.bits, regdp_format.rd.index); 
	*toReturn |= place_bits(rn, regdp_format.rn.bits, regdp_format.rn.index); 
	*toReturn |= place_bits(ra, regdp_format.ra.bits, regdp_format.ra.index); 
	*toReturn |= place_bits(rm, regdp_format.rm.bits, regdp_format.rm.index); 	
	 
	return EXIT_SUCCESS; 
}

int compare(char **params, int numparams, uint32_t *toReturn) {
	printf("debug: this is a compare/test expression\n"); 
	char *zero = strchr(params[1], 'x') ? "xzr" : "wzr";
	
	// Insert the zero register at params[1]
	for (int i = numparams; i > 1; i--) {
	   params[i] = params[i - 1];  
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
	// Set destination to the zero register 
	*toReturn |= ZRSP; 
	return EXIT_SUCCESS; 
}
