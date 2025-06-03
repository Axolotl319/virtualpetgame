#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h" 
#include "modify-regs.h"
#include "data-processing.h"
#include "branch.h"
#include "data-transfer.h"
#include "extract-bits.h"
#include "instr-formats.h"
#include "constants.h"
 
#define HALT 0x8a000000 //halt instruction
#define OPT_ARGS 3      //optional number of args for main
#define ARG_INPUT 1    //argument for input file
#define ARG_OUTPUT 2    //argument for output file

//initialise the registers and memory to 0. Set PSTATE Z flag to 1.
static void initialise(armv8_state *armv8) {
	memset(armv8, 0, sizeof(*armv8));
	armv8->PSTATE.Z = true;
}

//print the armv8 state
static void print_state(armv8_state *armv8, FILE *outFile) {
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
	for (int addr = 0; addr < (MEM_SIZE - WORD_SIZE_32); addr+=WORD_SIZE_32) {
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
// Returns 1 if halting condition reached. 
// Returns 2 if branch statement.  
// Returns -1 if decoding unsuccessful, returns 0 if successful.  
static int decode(uint8_t *instruction, armv8_state *armv8) {
	
	// Combines four consecutive bytes to 32 bits, taking into account little endian  
	uint32_t result = ((uint32_t) *(instruction+3) << 24) 
		| ((uint32_t) *(instruction+2) << 16) 
		| ((uint32_t) *(instruction+1) << 8) 
		| ((uint32_t) *instruction);

	// Checks for halting instruction 
	if (result == HALT) { return 1; }

	// Obtain op0 
	unsigned int opzero = extract_bits(result, OP0_INDEX, OP0_BITS);	
	
	//Type for load/store and load literal
	int type = extract_bits(result, sdt_format.type.index, sdt_format.type.index);

	int branchStat; //branch status	

	int op0_group = get_op0_group(opzero); //get the instr type 

	switch (op0_group) {
		case IMMDP_GROUP: //Data processing (immediate) 
			return immdp( result, armv8 ) ? -1 : 0;
		
		case REGDP_GROUP: //Data processing (registers)
			return regdp( result, armv8 ) ? -1 : 0;
		  
		case LDSTR_GROUP: //Load/Store
			return type ? //Load/Store with offset
				datatransfer( result, armv8 ) ? -1 : 0 : 
			        //Load Literal 
				loadliteral( result, armv8 ) ? -1 : 0;

		case BR_GROUP: //Branch
		        branchStat = branch( result, armv8 ); 	
			if (branchStat == -1) {
				return -1; 
			} else if (branchStat) {
				return 2; 
			} 
			return 0; 
		default: 
			fprintf(stderr, "Bad opcode (op0).\n");
			return -1;
	}
}	



//Fetches instructions based on PC, passes each instruction to decode
//Returns 1 if unsuccessful, 0 if successful
static int fetch(armv8_state *armv8) {
 
	int status = 0;

	// Decodes each instruction  
	while(1) {
		uint8_t *current = &armv8->memory[armv8->PC];		
		
		//if PC is out of bounds
		if (armv8->PC + WORD_SIZE_32 > MEM_SIZE) {
			fprintf(stderr, "PC out of bounds\n");
			return 1;
		}

		status = decode(current, armv8);
		
		printf("Status code: %d\n", status); 

		if ( status == 1 ) { 	//HALT
			break;
		}
		if ( status < 0 ) {	//Decode failed
			return 1; 
		}

		if ( status == 0 ) {	//Increment PC if decode success and not branch
			incrementPC(armv8); 
		} 
	}
	return 0;
}

int main(int argc, char **argv) 
{
	//create an armv8 state and initialise the registers
	armv8_state armv8;
	initialise(&armv8);

	//argc = 2 or 3, ./emulate is 1st arg
  	FILE *inFile = fopen(argv[ARG_INPUT], "rb");
	FILE *outFile;
	if(inFile == NULL){
		perror("Couldn't open input file.");
		return 1;
	}

	if(argc >= OPT_ARGS){
		outFile = fopen(argv[ARG_OUTPUT], "w");
	}else{
		outFile = stdout;
	}

	if(outFile == NULL) {
		perror("Couldn't open output file.");
		return 1;
	}
	
	armv8.memory = malloc(MEM_SIZE);
	if(armv8.memory == NULL){
		fprintf(stderr, "Couldn't allocate buffer memory");
		fclose(inFile);
		return 1;
	}
	
	// Reads contents of file into buffer 
	fread(armv8.memory, 1, MEM_SIZE, inFile);	
	fclose(inFile);

	//Calls fetch function, which calls decode
	if (fetch(&armv8)) {
		fprintf(stderr, "Couldn't execute the instruction\n");
		return 1;
	}
	
	//print armv8 state
	print_state(&armv8, outFile);

	free(armv8.memory);
	
	return EXIT_SUCCESS;
}
