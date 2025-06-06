#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
//#include "armv8.h"
#include "symtable.h"
//#include "constants.h"

#define MAXLINELEN 256
#define WORD_SIZE_32 4

int main(int argc, char **argv) {

	if (argc != 3) {
		fprintf(stderr, "Incorrect arguments given.\n");
	        return EXIT_FAILURE; 	
	}
	
	// Open the input file 
	FILE *filein = fopen(argv[1], "r"); 
	char linein[MAXLINELEN]; 
	
	// First pass: store labels and addresses in symbol table
        symbol_table symtable = emptyST( );	

	// A regular expression to help identify labels: 
	regex_t regex; 
	//int reti = regcomp(&regex, "_start", 0); 
	int reti = regcomp(&regex, "[a-zA-Z_.]([a-zA-Z0-9$_.])*:", REG_EXTENDED); 
	if (reti) {
		fprintf(stderr, "Regex could not be compiled.\n"); 
		return EXIT_FAILURE; 
	}
	
	//uint8_t *addr = armv8_state->memory; //from armv8.h, unsure how to link, may need to put in utils?
	uint8_t addr = 0; 
	while(fgets(linein, MAXLINELEN, filein)) { 
		int len = strlen(linein); 
		if (len > 0 && linein[len-1] == '\n') {
			linein[len-1] = '\0'; 
		} 
		reti = regexec(&regex, linein, 0, NULL, 0);  
		if (!reti) { 
			addPair(symtable, linein, &addr);
			printf("debug: Label: %s, address: %d\n", linein, *getAddress(symtable, linein));
		}
		addr+=WORD_SIZE_32; 
	}

	// Second pass: generate binary encoding
	

	// Clean up and prepare to exit 
	fclose(filein); 
	freeST(symtable);
        	
	return EXIT_SUCCESS;
}
