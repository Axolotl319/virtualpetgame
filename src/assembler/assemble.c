#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include "symtable.h"

#define MAXLINELEN 256 

int main(int argc, char **argv) {

	if (argc != 3) {
		fprintf(stderr, "Incorrect arguments given.\n");
	        return EXIT_FAILURE; 	
	}
	
	// Open the input file 
	FILE *filein = fopen(argv[1], "r"); 
	char linein[MAXLINELEN]; 
	
	// First pass: store labels and addresses in symbol table
	// May be preferable to make labels and addresses dynamic arrays  		
	char **labels = malloc(15 * sizeof(char *));
	uint8_t *addresses = malloc(15 * sizeof(uint8_t));
	int nelements = 0;

	// A regular expression to help identify labels: 
	regex_t regex; 
	//int reti = regcomp(&regex, "_start", 0); 
	int reti = regcomp(&regex, "[a-zA-Z_.]([a-zA-Z0-9$_.])*:", REG_EXTENDED); 
	if (reti) {
		fprintf(stderr, "Regex could not be compiled.\n"); 
		return EXIT_FAILURE; 
	}
	
	int addr = 0; 
	while(fgets(linein, MAXLINELEN, filein)) { 
		int len = strlen(linein); 
		if (len > 0 && linein[len-1] == '\n') {
			linein[len-1] = '\0'; 
		} 
		reti = regexec(&regex, linein, 0, NULL, 0);  
		if (!reti) { 
			labels[nelements] = strdup(linein); 
			addresses[nelements] = addr; 
			nelements++; 
		}
		addr+=4; 
	}

	symbol_table *symtable = createST(labels, addresses, nelements);
	// For debugging purposes: 
	for(int i=0; i<15; i++) {
		printf("Label: %s, address: %u\n", labels[i], addresses[i]); 
	}
	
	// Second pass: generate binary encoding
	

	// Clean up and prepare to exit 
	fclose(filein); 
	freeST(symtable); 
	
	return EXIT_SUCCESS;
}
