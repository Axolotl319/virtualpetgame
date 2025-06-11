#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "symtable.h"
#include "constants.h"
#include "aliases.h"

#define NUM_ARGS 3
#define MAXLINELEN 256
#define MAX_PARAMS 5
#define LABEL_REGEX "[a-zA-Z_.]([a-zA-Z0-9$_.])*:"

static regex_t label_regex;

//compiles regex 
static int compile_regex( void ) {
	// A regular expression to help identify labels:  
	int reti = regcomp(&label_regex, LABEL_REGEX, REG_EXTENDED); 
	if (reti) {
		fprintf(stderr, "Regex could not be compiled.\n"); 
		return EXIT_FAILURE; 
	}
	assert(!reti);
	return EXIT_SUCCESS;
}

//checks if line is label
//returns true if label, false if not
static bool is_label(char *linein) {
	int reti = regexec(&label_regex, linein, 0, NULL, 0);
	return !reti;
}

//checks if line is an int directive
static bool is_int_directive(char *linein) {
	return !(strncmp(linein, ".int", 4));
}

//checks if line is an empty line
static bool is_empty_line(const char *linein) {
	while (*linein != '\0') {
		if (!isspace(*linein)) {
			return false;
		}
		linein++;
	}
	return true;
}	

//strips newline from end of line
static void strip_newline(char *linein) {
	int len = strlen(linein);
        if (len > 0 && linein[len - 1] == '\n') {
		linein[len - 1] = '\0';
        }
}

//tokenises the instruction
static void get_instr_params(char *instr, char **params, int *numparams) { 
   char *rest = NULL; 
   params[0] = strtok_r(instr, " ", &rest);
   char *param = strtok_r(NULL, ",", &rest);  
   while (param != NULL) {
	// Remove all whitespace from the front of any parameters: 
	while (isspace(*param)) {
		param++; 
	}
	params[*numparams] = param;  
   	param = strtok_r(NULL, ",", &rest);
	(*numparams)++; 
   }
}

//replaces labels with addresses from symtable
//replaces labels with decimal address in the form of a string
static void replace_labels(symbol_table symtable, char **params, int numparams, uint32_t current_addr) {
	for (int i = 1; i < numparams; i++) {
		printf("DEBUG: To search: %s\n", params[i]);
		uint32_t address = getAddress(symtable, params[i]);
		if (address == 1) { continue; }
		printf("DEBUG: Label Address: 0x%x\n", address);
		printf("DEBUG: Current Address: 0x%x\n", current_addr);
		int32_t offset = address - current_addr;
		printf("DEBUG: Offset: %d\n", offset);
		sprintf(params[i], "%d", offset);
	}
}


// First pass: store labels and addresses in symbol table
static int first_pass(symbol_table symtable, FILE* filein) {
	char linein[MAXLINELEN];
	
	uint32_t addr = 0; 
	while(fgets(linein, MAXLINELEN, filein)) { 
		//skip a newline
		if (*linein == '\n') { continue; }

		//strip newline character from end of line
		strip_newline(linein);

		if (is_empty_line(linein)) { continue; }

		if (is_label(linein)) {
			// Remove the colon
			for (int i=strlen(linein)-1; i>=0; i--) {
				if (linein[i] == ':') {
					linein[i] = '\0'; 
					break;
				}
			}
			if (addPair(symtable, linein, addr)) { return EXIT_FAILURE; }
		} else {
			addr += WORD_SIZE_32; 
		}
	}
	return EXIT_SUCCESS;
	
}

//Second pass: reads each instruction and int directive
//Calls functions to generate binary code
//Replaces label references with addresses from symtable
static int second_pass(symbol_table symtable, FILE* filein) {
	//reread file
	char linein[MAXLINELEN];
	char *line = linein;
	uint8_t addr = 0;
	
	while(fgets(line, MAXLINELEN, filein)) {
		//skip new line 
		if (*line == '\n') { continue; }

		//strip newline character from end of line
		strip_newline(line);

		//check if it is an empty line
		if (is_empty_line(line)) { continue; }

		//if line is label, ignore it
		if (is_label(line)) { continue; }

		//strip spaces from start of line
		while (*line == ' ' || *line == '\t') {
			line++;
		}

		//int directive
		if (is_int_directive(line)) {
			printf("DEBUG: Int directive: %s\n", line);
			//CALL FUNCTION TO PARSE
		} else { 
			printf("DEBUG: Instruction: %s\n", line);
			char tok[10]; 	
			if (!sscanf(line, "%s", tok)) {
				fprintf(stderr, "Instruction read failed.\n"); 
				return EXIT_FAILURE;
			}	
			if (strncmp(tok, "b.", 2) == 0) {
				strcpy(tok, "b."); 
			}
			parse_f pf = lookup_alias(tok);
		        if (pf == NULL) {
				fprintf(stderr, "Invalid instruction.\n");
				return EXIT_FAILURE; 
			}
			
			//gets array of operands
			char *params[MAX_PARAMS];
			for (int i = 0; i < MAX_PARAMS; i++) { params[i] = NULL; }
        		int numparams = 1; 	
			get_instr_params(line, params, &numparams); 
			
			//Replaces label names with addresses
                        replace_labels(symtable, params, numparams, addr);

			//Call function to parse and store result in variable "tobin"
			uint32_t tobin = pf(params, numparams); 
			printf("debug: To convert to binary: %x\n", tobin); 			
			if (!tobin) {
				fprintf(stderr, "Instruction parse failed.\n");
			        return EXIT_FAILURE; 	
			}

			
		}

		addr += WORD_SIZE_32;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {

	if (argc != NUM_ARGS) {
		fprintf(stderr, "Incorrect arguments given.\n");
	        return EXIT_FAILURE; 	
	}
	assert(argc == NUM_ARGS);
	
	// Open the input file 
	FILE *filein = fopen(argv[1], "r");
       	if (filein == NULL) { 
		fprintf(stderr, "Unable to open file\n");
		return EXIT_FAILURE;
	}	
	assert(filein != NULL);

	
	// First pass: store labels and addresses in symbol table
        symbol_table symtable = emptyST();
	if (symtable == NULL) { return EXIT_FAILURE; }
	assert(symtable != NULL);

	//Compile regex for labels
	if (compile_regex()) { return EXIT_FAILURE; }

	if (first_pass(symtable, filein)) { return EXIT_FAILURE; }

	printf("debug: symbol table pairs listed below:\n"); 
	for (int i=0; i<symtable->length; i++) {
		symbol_pair p = symtable->st_pairs[i]; 
		printf("Pair %s, %d\n", p->label, p->address); 
	}

	//rewind file to go back to start
	rewind(filein);

	// Second pass: generate binary encoding
	if (second_pass(symtable, filein)) { return EXIT_FAILURE; }

	// Clean up and prepare to exit 
	fclose(filein);
	regfree(&label_regex);	
	freeST(symtable);
        	
	return EXIT_SUCCESS;
}
