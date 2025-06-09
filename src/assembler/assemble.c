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

// First pass: store labels and addresses in symbol table
static int first_pass(symbol_table symtable, FILE* filein) {
	char linein[MAXLINELEN];
	
	uint8_t addr = 0; 
	while(fgets(linein, MAXLINELEN, filein)) { 
		//skip a newline
		if (*linein == '\n') { continue; }

		//strip newline character from end of line
		strip_newline(linein);

		if (is_label(linein)) { 
			if (addPair(symtable, linein, &addr)) { return EXIT_FAILURE; }
			printf("debug: Label: %s, address: %d\n", linein, *getAddress(symtable, linein));
		}

		addr += WORD_SIZE_32; 
	}
	return EXIT_SUCCESS;
	
}

//Second pass: reads each instruction and int directive
//Calls functions to generate binary code
//Replaces label references with addresses from symtable HAVENT DONE THIS YET
//will do after tokeniser is implemented
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
			printf("Int directive: %s\n", line);
			//CALL FUNCTION TO PARSE
		} else { 
			printf("Instruction: %s\n", line); 	
			char *tok = strtok(line, " "); 
			if (*tok == 'b') {
				tok = "b"; 
			}
			parse_f pf = lookup_alias(tok);
		        if (pf == NULL) {
				fprintf(stderr, "Invalid instruction.\n");
				return EXIT_FAILURE; 
			}	
			//CALL FUNCTION TO PARSE
			if (!pf(line)) {
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

	//rewind file to go back to start
	rewind(filein);

	// Second pass: generate binary encoding
	if (second_pass(symtable, filein)) { return EXIT_FAILURE; }

	// Clean up and prepare to exit 
	fclose(filein); 
	freeST(symtable);
        	
	return EXIT_SUCCESS;
}
