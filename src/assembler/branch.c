#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "instr-formats.h"
#include "constants.h"
#include "assembly-utils.h"
#include "branch.h"

#define NUM_CONDS      7
#define BR_UNCOND_BASE 0x14000000
#define BR_REG_BASE    0xD61F0000
#define BR_COND_BASE   0x54000000
#define MAX_19_BITS    (1 << 18)
#define MAX_26_BITS    (1 << 25)
#define NUM_BR_PARAMS  2

static cond_pair branch_conds[] = {
	{"al", BR_AL},
	{"eq", BR_EQ},
	{"ge", BR_GE},
	{"gt", BR_GT},
	{"le", BR_LE},
	{"lt", BR_LT},
	{"ne", BR_NE}
};

static int cond_pair_cmp(const void *key, const void *elem) {
	const char *cond_str = (const char *)key;
	const cond_pair *pair = (const cond_pair *)elem;
	return strcmp(cond_str, pair->cond_str);
}

static int check_numparams(int numparams) {
	if (numparams != NUM_BR_PARAMS) {
		fprintf(stderr, "Invalid branch instruction\n");
		return EXIT_FAILURE;
	}
	assert(numparams == NUM_BR_PARAMS);
	return EXIT_SUCCESS;
}

// Register branch
// Puts result into int pointer
// Returns 1 for failure, 0 for success
int reg_branch(char **params, int numparams, uint32_t *instr) {
	assert(params != NULL);
	assert(instr != NULL);
	
	if (check_numparams(numparams)) { return EXIT_FAILURE; }

	*instr = BR_REG_BASE;
	
	//get xn
	uint8_t xn = obtain_reg_num(params[1]);

	//form instr by masking + shifting xn:
	
	*instr |= place_bits(xn, br_format.xn.bits, br_format.xn.index); 
	return EXIT_SUCCESS;		
}

// Unconditional Branch
int uncond_branch(char **params, int numparams, uint32_t *instr) {
	assert(params != NULL);
	assert(instr != NULL);

	if (check_numparams(numparams)) { return EXIT_FAILURE; }

	*instr = BR_UNCOND_BASE;
	
	//get simm26
	int32_t simm26 = strtol(params[1], NULL, 10) / WORD_SIZE_32;

	//check valid range
	if (simm26 < -(MAX_26_BITS) || simm26 > MAX_26_BITS - 1) { 
		fprintf(stderr, "Offset not in  range\n");
		return EXIT_FAILURE;
	}
	assert(simm26 >= -(MAX_26_BITS) && simm26 <= MAX_26_BITS - 1);

	//form the instruction by masking + shifting simm26
	*instr |= place_bits(simm26, 
			     br_format.simm26.bits, 
			     br_format.simm26.index);
		
	return EXIT_SUCCESS;
}

// Conditional Branch
int cond_branch(char **params, int numparams, uint32_t *instr) {
	assert(params != NULL);
	assert(instr != NULL);
	
	if (check_numparams(numparams)) { return EXIT_FAILURE; }

	*instr = BR_COND_BASE;

	//check if instruction is valid
	char *cond_str = strchr(params[0], '.');
	if (cond_str != NULL && *(cond_str + 1) != '\0') {
		cond_str++;
	} else {
		fprintf(stderr, "Invalid branch condition\n");
		return EXIT_FAILURE;
	}
	
	cond_pair *pair = bsearch(cond_str, 
				  branch_conds, 
				  NUM_CONDS, 
				  sizeof(cond_pair), 
				  cond_pair_cmp);

	if (pair == NULL) { 
		fprintf(stderr, "Invalid branch condition\n");
		return EXIT_FAILURE;
	}
	assert(pair != NULL);

	//get the condition int
	int cond = pair->bcond;

	//get simm19
	int32_t simm19 = strtol(params[1], NULL, 10) / WORD_SIZE_32;

	//check valid range
	if (simm19 < -(MAX_19_BITS) || simm19 > MAX_19_BITS - 1) { 
		fprintf(stderr, "Offset not in  range\n");
		return EXIT_FAILURE;
	}
	assert(simm19 >= -(MAX_19_BITS) && simm19 <= MAX_19_BITS - 1);

	//form the instruction by masking + shifting simm19 and cond
	*instr |= place_bits(simm19, 
			     br_format.simm19.bits, 
			     br_format.simm19.index);

	*instr |= place_bits(cond, 
			     br_format.cond.bits, 
			     br_format.cond.index); 

	return EXIT_SUCCESS;
}
