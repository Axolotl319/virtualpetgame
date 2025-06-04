#include <stdlib.h>
#include <stdio.h>
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
	
	// First pass: read each line of the assembly file
	// Stores labels and addresses  		
	char **labels;
	uint8_t *addresses;
	int nelements = 0;

	while(fgets(linein, MAXLINELEN, filein)) {
		printf("%s", linein); 
	}

	symbol_table *symtable = createST(labels, addresses, nelements);

	// Clean up and prepare to exit 
	fclose(filein); 
	freeST(symtable); 
	
	return EXIT_SUCCESS;
}
