#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "data-processing.h"
#include "assembly-utils.h"
#include "instr-formats.h"
#include "constants.h"

//base instructions
#define IMM_BASE       0x10000000
#define REG_BASE       0x0A000000
#define LOGIC_BASE     0x0A000000
#define MOV_BASE       0x12800000
#define MUL_BASE       0x1b000000
#define ARITH_OPR_BASE 0x8
#define OPI_ARITH      0x2

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

//shift codes
static const code_map shift_codes[] = {
	{"lsl", LSL, 0},
	{"lsr", LSR, 0},
	{"asr", ASR, 0},
	{"ror", ROR, 0},
	{ NULL, 0  , 0}
};

//logic instructions opcodes
static const code_map logic_opcs[] = {
	{"and" , LOG_AND , 0},
	{"bic" , LOG_AND , 1},
	{"orr" , LOG_OR  , 0},
	{"orn" , LOG_OR  , 1},
	{"eor" , LOG_XOR , 0},
	{"eon" , LOG_XOR , 1},
	{"ands", LOG_ANDS, 0},
	{"bics", LOG_ANDS, 1},
	{ NULL , 0       , 0}
};

//arithmetic instruction opcodes
static const code_map arith_opcs[] = {
	{"add" , ARITH_ADD , 0},
	{"adds", ARITH_ADDS, 0},
	{"sub" , ARITH_SUB , 0},
	{"subs", ARITH_SUBS, 0},
	{ NULL , 0         , 0}
};

//move instruction opcodes
static const code_map mov_opcs[] = {
	{"movk", MOVK, 0},
	{"movz", MOVZ, 0},
	{"movn", MOVN, 0},
	{ NULL , 0   , 0}
};

//single operand destination map
static const func_map sop_functions[] = {
	{"mul" , "madd", &multiply, 0},
	{"mneg", "msub", &multiply, 0},
	{"mov" , "orr" , &logic   , 1},
	{"mvn" , "orn" , &logic   , 1},
	{"neg" , "sub" , &arith   , 1},
	{"negs", "subs", &arith   , 1},
	{ NULL , NULL  , NULL     , 0}
};

static const func_map cmp_functions[] = {
	{"tst", "ands", &logic, 0},
	{"cmp", "subs", &arith, 0},
	{"cmn", "adds", &arith, 0},
	{ NULL,  NULL , NULL  , 0}
};

//sets a single code for an instruction
//Used by: arithmetic instructions, shift type
static int set_codes(char *instr, int *code, int *n, const code_map *codes) {
	for(int i = 0; codes[i].instr != NULL; i++) {
		if (!strcmp(instr, codes[i].instr)) {
			*code = codes[i].code;
			if (n != NULL) { *n = codes[i].n; }
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

//returns the parse function for single operand destination function
//also replaces an operand with the zero register
static parse_f redirect_func(char **params, int numparams, const func_map *funcs) {
	for (int i = 0; funcs[i].instr != NULL; i++) {
		if (strcmp(params[0], funcs[i].instr)) { continue; }
		
		params[0] = funcs[i].alias;
		
		//deal with compare function
		if (numparams == 0) {
			return funcs[i].pf;
		}

		//single operand destination function - replace with zr
		if (funcs[i].insert_xzr) {
			insert_zero_reg(numparams);
		} else { //insert it as the last parameter
			params[numparams++] = "xzr";
		}
			
		return funcs[i].pf;
	}
	return NULL;
}

// Sets the bits for shift type and shift amount in instr. 
// Returns true for failure and false for success
static int set_shift_info(uint32_t *instr, bool logic, char *param) {
	//read in shift type
	char shifttype[4];
	if (sscanf(param, "%s", shifttype) == EOF) {
		fprintf(stderr, "Shift type could not be read.\n"); 
		return EXIT_FAILURE; 
	}	
	uint8_t oper = obtain_shift_amt(param);

	//get shift type code
	int shift;
	if (set_codes(shifttype, &shift, NULL, shift_codes)) {
		fprintf(stderr, "Shift type not recognised.\n");
		return EXIT_FAILURE;
	}

	*instr |= place_bits(shift, regdp_format.shift.bits, regdp_format.shift.index);
	*instr |= place_bits(oper, regdp_format.operand.bits, regdp_format.operand.index);
        return EXIT_SUCCESS; 	
}

// Parses arithmetic instructions into decimal format 
int arith(char **params, int numparams, uint32_t *instr) {
        if (numparams != MIN_AL_PARAMS && numparams != MAX_AL_PARAMS) {
		fprintf(stderr, "Incorrect number of parameters.\n"); 
		return EXIT_FAILURE; 
	}
	assert(numparams == MIN_AL_PARAMS || numparams == MAX_AL_PARAMS);

	*instr = 0; 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 

	*instr |= place_bits(rd, immdp_format.rd.bits, immdp_format.rd.index); 
	*instr |= place_bits(rn, immdp_format.rn.bits, immdp_format.rn.index); 

	update_sf(instr, immdp_format.sf.index, params[2]); 

	// Update opc
	int opc;
	if(set_codes(params[0], &opc, NULL, arith_opcs)) {
		fprintf(stderr, "Unrecognised arithmetic instruction\n");
		return EXIT_FAILURE;
	}

	*instr |= place_bits(opc, immdp_format.opc.bits, immdp_format.opc.index);

	if (is_imm(params[3])) {
		// Then it is immediate value arithmetic
		// Set the constant base at bit 28 
		*instr |= IMM_BASE;

		// Update opi (binary 010) 
		*instr |= place_bits(OPI_ARITH, immdp_format.opi.bits, immdp_format.opi.index);
		
		// Obtain imm12 value 
		*instr |= place_bits(extract_imm(params[3]), immdp_format.imm12.bits, immdp_format.imm12.index);

		// Update shift bit if needed
		if (numparams == MAX_AL_PARAMS && obtain_shift_amt(params[4]) == IMM12_LEN) {
		   *instr |= place_bits(1, immdp_format.sh.bits, immdp_format.sh.index);
		}
	} else {
		// Else it is register arithmetic
		// Set constant bases at bits 27 and 25 
		*instr |= REG_BASE;
		
		uint8_t rm = obtain_reg_num(params[3]); 
		*instr |= place_bits(rm, regdp_format.rm.bits, regdp_format.rm.index); 

		// Update opr (binary 1000)
		*instr |= place_bits(ARITH_OPR_BASE, regdp_format.opr.bits, regdp_format.opr.index); 

		// Update shift and operand if needed
		if (numparams == MAX_AL_PARAMS) {
			if (set_shift_info(instr, false, params[4])) {
				return EXIT_FAILURE; 
			}		
		}
	} 
	return EXIT_SUCCESS; 
}

int logic(char **params, int numparams, uint32_t *instr) {
	if (numparams != MIN_AL_PARAMS && numparams != MAX_AL_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n");
	        return EXIT_FAILURE; 	
	}
	assert(numparams == MIN_AL_PARAMS || numparams == MAX_AL_PARAMS);

	// Set up the instruction base 
	*instr = LOGIC_BASE;

	update_sf(instr, regdp_format.sf.index, params[1]); 

	// Obtain and update all the registers 
	uint8_t rd = obtain_reg_num(params[1]); 
	uint8_t rn = obtain_reg_num(params[2]); 
	uint8_t op2;
	if (is_imm(params[3])) {
		op2 = extract_imm(params[3]);
	} else {
        	op2 = obtain_reg_num(params[3]);
	}

        *instr |= place_bits(rd, regdp_format.rd.bits, regdp_format.rd.index); 
	*instr |= place_bits(rn, regdp_format.rd.bits, regdp_format.rn.index); 
	*instr |= place_bits(op2, regdp_format.rm.bits, regdp_format.rm.index); 

	// Set opc and N (negate flag) depending on the mnemonic 
	int opc; 
	int n;
	if (set_codes(params[0], &opc, &n, logic_opcs)) { 
		fprintf(stderr, "Unrecognised logic instruction\n");
		return EXIT_FAILURE; 
	}

	*instr |= place_bits(opc, regdp_format.opc.bits, regdp_format.opc.index); 
	*instr |= place_bits(n, regdp_format.N.bits, regdp_format.N.index); 

	// Set the shift type if needed
	if (numparams == MAX_AL_PARAMS) {
		if (set_shift_info(instr, true, params[4])) {
			return EXIT_FAILURE; 
		}	
	}
	 
	return EXIT_SUCCESS; 
}

int wmove(char **params, int numparams, uint32_t *instr) {
        if (numparams != MIN_MOV_PARAMS && numparams != MAX_MOV_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n");
	        return EXIT_FAILURE; 	
	}
	// Set the instruction base 
	*instr = MOV_BASE;

	update_sf(instr, immdp_format.sf.index, params[1]); 

	// Update rd 
	*instr |= place_bits(obtain_reg_num(params[1]), immdp_format.rd.bits, immdp_format.rd.index); 

	// Update opc
	int opc;
	if (set_codes(params[0], &opc, NULL, mov_opcs)) {
		fprintf(stderr, "Unrecognised wide move mnemonic.\n");
		return EXIT_FAILURE;
	}
	*instr |= place_bits(opc, immdp_format.opc.bits, immdp_format.opc.index);

	// Extract imm16 and update instr 
	*instr |= place_bits(extract_imm(params[2]), immdp_format.imm16.bits, immdp_format.imm16.index);

	// If a left shift exists, update the instruction 
	if (numparams == MAX_MOV_PARAMS) {
		uint8_t bit = obtain_shift_amt(params[3]) / IMM16_LEN; 
		*instr |= place_bits(bit, immdp_format.hw.bits, immdp_format.hw.index);
	}
 
	return EXIT_SUCCESS; 
}


int single_op_dest(char **params, int numparams, uint32_t *instr) {
	if (numparams != MIN_SOP_PARAMS && numparams != MAX_SOP_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n"); 
		return EXIT_FAILURE; 
	}
	
	parse_f pf = redirect_func(params, numparams, sop_functions);

	if (pf == NULL) {
		fprintf(stderr, "Unrecognized mnemonic.\n");
		return EXIT_FAILURE;
	}
	assert(pf != NULL);

	numparams++;
	pf(params, numparams, instr);

	return EXIT_SUCCESS; 
}

int multiply(char **params, int numparams, uint32_t *instr) {
        if (numparams != MUL_PARAMS) {
		fprintf(stderr, "Invalid number of parameters.\n"); 
		return EXIT_FAILURE; 
	}
	assert(numparams == MUL_PARAMS);

	// Set the instruction base 
	*instr = MUL_BASE; 
 
	update_sf(instr, regdp_format.sf.index, params[1]); 

	// Update x: 
	if (!strcmp(params[0], "msub")) {
		*instr |= place_bits(1, regdp_format.x.bits, regdp_format.x.index); 
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

	*instr |= place_bits(rd, regdp_format.rd.bits, regdp_format.rd.index); 
	*instr |= place_bits(rn, regdp_format.rn.bits, regdp_format.rn.index); 
	*instr |= place_bits(ra, regdp_format.ra.bits, regdp_format.ra.index); 
	*instr |= place_bits(rm, regdp_format.rm.bits, regdp_format.rm.index); 	
	 
	return EXIT_SUCCESS; 
}

int compare(char **params, int numparams, uint32_t *instr) {
	char *zero = strchr(params[1], 'x') ? "xzr" : "wzr";
	
	// Insert the zero register at params[1]
	for (int i = numparams; i > 1; i--) {
	   params[i] = params[i - 1];  
	}
	params[1] = zero; 
	numparams++;

	parse_f pf = redirect_func(params, 0, cmp_functions);
	if (pf == NULL) {
		fprintf(stderr, "Unrecognized compare mnemonic.\n");
		return EXIT_FAILURE;
	}	
	pf(params, numparams, instr);

	// Set destination to the zero register 
	*instr |= ZRSP; 
	return EXIT_SUCCESS; 
}
