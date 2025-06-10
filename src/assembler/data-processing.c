#include <stdio.h>
#include <stdlib.h> 
#include "data-processing.h"
#include "assembly_utils.h"

// Returns the encoded instruction if success, -1 if fail
int arith(char *instr) {
	printf("debug: this is an arithmetic expression\n"); 
	return 1; 
}

int logic(char *instr) {
	printf("debug: this is a logic expression\n"); 
	return 1; 
}

int wmove(char *instr) {
	printf("debug: this is a wide move expression\n"); 
	return 1; 
}

int single_op_dest(char *instr) {
	printf("debug: this is a single op and destination expression\n"); 
	return 1; 
}

int multiply(char *instr) {
	printf("debug: this is a multiply expression\n"); 
	return 1; 
}

int compare(char *instr) {
	printf("debug: this is a compare/test expression\n"); 
	return 1; 
}
