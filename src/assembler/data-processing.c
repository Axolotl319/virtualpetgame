#include <stdio.h>
#include <stdlib.h> 
#include "data-processing.h"
#include "assembly_utils.h"

// Return 1 if success, 0 if fail
int dp(char *instr) {
	printf("debug: This is a data processing instruction\n"); 
	char **params = malloc(10 * sizeof(char *));
        int numparams = 0; 	
	get_instr_params(instr, params, &numparams); 
	return 1;
}
