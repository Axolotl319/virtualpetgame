#include <stdint.h>
#include <stdio.h>
#include "assembly-utils.h"

uint32_t int_directive(char **params, int numparams) {
	if (numparams != 2) {
		fprintf(stderr, "Invalid int directive\n");
		return 0;
	}

	char *value = params[1];
	uint32_t val_to_write = extract_imm(value);
	return val_to_write;
}
