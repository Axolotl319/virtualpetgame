#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"
#include "modify-regs.h"
#include "armv8.h" 

//initialise the registers and memory to 0. Set PSTATE Z flag to 1.
void initialise(armv8_state *armv8) {
	memset(armv8, 0, sizeof(*armv8));
	armv8->PSTATE.Z = true;
}

//print the armv8 state
void print_state(armv8_state *armv8, FILE *outFile) {
	//General Purpose Registers
	fprintf(outFile, "Registers:\n");
	for (int i = 0; i < NUM_GP_REGS; i++) {	
		fprintf(outFile, "X%02d = %016lx\n", i, armv8->GP_regs[i]);
	}

	//PC
	fprintf(outFile, "PC  = %016lx\n", armv8->PC);

	//PSTATE
	fprintf(outFile, "PSTATE: %c%c%c%c\n", 
			armv8->PSTATE.N ? 'N' : '-',
			armv8->PSTATE.Z ? 'Z' : '-',
			armv8->PSTATE.C ? 'C' : '-',
			armv8->PSTATE.V ? 'V' : '-');

	//Memory
	fprintf(outFile, "Non-zero memory:\n");
	for (int addr = 0; addr < (MEM_SIZE - WORD_SIZE); addr+=WORD_SIZE) {
		uint32_t word = 
			((uint32_t)armv8->memory[addr])
			| ((uint32_t)armv8->memory[addr + 1] << 8)
			| ((uint32_t)armv8->memory[addr + 2] << 16)
			| ((uint32_t)armv8->memory[addr + 3] << 24);

		if (word != 0) {
			fprintf(outFile, "0x%08x: 0x%08x\n", addr, word);
		}
	}

}

// Given the starting address of an instruction, determines the instruction type 
// and passes the instruction to the corresponding function to handle. 
// Returns 1 if decoding unsuccessful, returns 0 if successful.
// Returns -1 if halting instruction reached or decoding unsuccessful, returns 0 if successful.  
int decode(uint8_t *instruction) {
	
	// Combines four consecutive bytes to 32 bits, taking into account little endian  
	uint32_t result = ((uint32_t) *(instruction+3) << 24) 
		| ((uint32_t) *(instruction+2) << 16) 
		| ((uint32_t) *(instruction+1) << 8) 
		| ((uint32_t) *instruction);

	// Checks for halting instruction 
	if (result == 0x8a000000) {
		return -1; 
	}

	// Obtain op0 -- comments used for debugging purposes. 
	int opzero = (result >> 25) & 0xf;
        printf("%u ", opzero);	
	switch (opzero) {
		case 8:
		case 9:
			printf("This is data processing (immediate).\n"); 
			immdp( result ); 
			break; 
		case 5:
		case 13:
			printf("This is data processing (registers).\n");
			regdp( result );
		        break; 
		case 4:
		case 6:
		case 12: 
		case 14: 
			printf("This is single data transfer.\n"); 
			int temp = 0xf & (result >> 31); 
			if (temp) {
				datatransfer( result ); 
			} else {
				loadliteral( result ); 
			}
			break; 
		case 10:
		case 11:
			printf("This is branch.\n"); 
			branch( result );
			break; 
		default: 
			fprintf(stderr, "Bad opcode (op0).\n");
			return -1;
			break; 
	}

	return 0; 
}	



//Fetches instructions based on PC, passes each instruction to decode
//Returns 1 if unsuccessful, 0 if successful
int fetch(armv8_state *armv8) {
 
	int status = 0;

	// Decodes each instruction  
	while(1) {
		uint8_t *current = &armv8->memory[armv8->PC];		
		
		//if PC is out of bounds
		if (armv8->PC + WORD_SIZE > MEM_SIZE) {
			fprintf(stderr, "PC out of bounds\n");
			return 1;
		}

		status = decode(current);
	
		if ( status == -1 ) { //HALT
			break;
		} else if ( status != 0) { //Decode failed
			return 1; 
		}

		//Increment PC - NEED TO CHANGE WHEN IMPLEMENTING BRANCH 	
		incrementPC(armv8); 
	}
	return 0;
}

int main(int argc, char **argv) 
{
	//create an armv8 state and initialise the registers
	armv8_state armv8;
	initialise(&armv8);

	//argc = 2 or 3, ./emulate is 1st arg
  	FILE *inFile = fopen(argv[1], "rb");
	FILE *outFile;
	if(inFile == NULL){
		perror("Couldn't open input file.");
		return 1;
	}

	if(argc >= 3){
		outFile = fopen(argv[2], "w");
	}else{
		outFile = stdout;
	}

	if(outFile == NULL) {
		perror("Couldn't open output file.");
		return 1;
	}
	
	armv8.memory = malloc(MEM_SIZE);
	if(armv8.memory == NULL){
		perror("Couldn't allocate buffer memory");
		fclose(inFile);
		return 1;
	}

	// Obtains size of file, kept in case needed later 
	// fseek(inFile, 0, SEEK_END); 
	// int filesize = ftell(inFile); 
	// rewind(inFile);
	
	// Reads contents of file into buffer 
	fread(armv8.memory, 1, MEM_SIZE, inFile);	
	fclose(inFile);

	//Calls fetch function, which calls decode
	if (fetch(&armv8)) {
		fprintf(stderr, "Couldn't execute the instruction\n");
	}
	
	//print armv8 state
	print_state(&armv8, outFile);

	free(armv8.memory);
	
	return EXIT_SUCCESS;
}
