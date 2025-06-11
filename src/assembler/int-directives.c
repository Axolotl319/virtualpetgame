#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "assembly-utils.h"

int int_directive(char **params, int numparams, uint32_t *toReturn) {
	if (numparams != 2) {
		fprintf(stderr, "Invalid int directive\n");
		return EXIT_FAILURE;
	}

	char *value = params[1];
	*toReturn = extract_imm(value);
	return EXIT_SUCCESS;
}
