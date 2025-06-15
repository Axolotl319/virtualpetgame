#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "assembly-utils.h"

#define NUM_PARAMS 2

int int_directive(char **params, int numparams, uint32_t *instr) {
	if (numparams != NUM_PARAMS) {
		fprintf(stderr, "Invalid int directive\n");
		return EXIT_FAILURE;
	}
	assert(numparams == NUM_PARAMS);

	char *value = params[1];
	*instr = extract_imm(value);
	return EXIT_SUCCESS;
}
