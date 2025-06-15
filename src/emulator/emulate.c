#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "constants.h"
#include "armv8.h" 
#include "data-processing.h"
#include "branch.h"
#include "data-transfer.h"
#include "emulator-utils.h"
#include "instr-formats.h"
#include "emulate.h"
#include <assert.h>
 
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
		uint64_t word;
		get_memory_data(armv8, addr, WORD_SIZE_32, &word);

		if (word != 0) {
			fprintf(outFile, "0x%08x: 0x%08x\n", addr, (uint32_t)word);
		}
	}

}

// Given the starting address of an instruction, determines the instruction type 
// and passes the instruction to the corresponding function to handle.
// Returns 1 if halting condition reached. 
// Returns 2 if branch statement.  
// Returns -1 if decoding unsuccessful, returns 0 if successful.  
static int decode(armv8_state *armv8) {
	
	// Combines four consecutive bytes to 32 bits, taking into account little endian  
	uint64_t temp;
       	if (get_memory_data(armv8, armv8->PC, WORD_SIZE_32, &temp)) { return DCD_FAIL; }
	uint32_t result = (uint32_t)(temp);

	// Checks for halting instruction 
	if (result == HALT) { return DCD_HLT; }

	// Obtain op0 
	unsigned int opzero = extract_bits(result, OP0_INDEX, OP0_BITS);	
	
	//Type for load/store and load literal
	int type = extract_bits(result, sdt_format.type.index, sdt_format.type.index);

	int branchStat; //branch status	

	int op0_group = get_op0_group(opzero); //get the instr type 

	switch (op0_group) {
		case IMMDP_GROUP: //Data processing (immediate) 
			return immdp( result, armv8 ) ? DCD_FAIL : DCD_SUCCESS;
		
		case REGDP_GROUP: //Data processing (registers)
			return regdp( result, armv8 ) ? DCD_FAIL : DCD_SUCCESS;
		  
		case LDSTR_GROUP: //Load/Store
			return type ? //Load/Store with offset
				datatransfer( result, armv8 ) ? DCD_FAIL : DCD_SUCCESS : 
			        //Load Literal 
				loadliteral( result, armv8 ) ? DCD_FAIL : DCD_SUCCESS;

		case BR_GROUP: //Branch
		        branchStat = branch( result, armv8 ); 	
			if (branchStat == BR_FAIL) {
				return DCD_FAIL; 
			} else if (branchStat == BR_SUCCESS) {
				return DCD_BRANCH; 
			} 
			return DCD_SUCCESS; 
		default: 
			fprintf(stderr, "Bad opcode (op0).\n");
			return DCD_FAIL;
	}
}	



//Fetches instructions based on PC, passes each instruction to decode
//Returns 1 if unsuccessful, 0 if successful
static int fetch(armv8_state *armv8) {
 
	int status = 0;

	// Decodes each instruction  
	while(1) {
		//if PC is out of bounds
		if (armv8->PC > MEM_SIZE - WORD_SIZE_32) {
			fprintf(stderr, "PC out of bounds\n");
			return EXIT_FAILURE;
		}	
		assert(armv8->PC <= MEM_SIZE - WORD_SIZE_32);

		status = decode(armv8); 

		if ( status == DCD_HLT ) { 	//HALT
			break;
		}
		if ( status == DCD_FAIL ) {	//Decode failed
			return EXIT_FAILURE; 
		}

		if ( status == DCD_SUCCESS ) {	//Increment PC if decode success and not branch
			incrementPC(armv8); 
		} 
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv) 
{

	//argc = 2 or 3, ./emulate is 1st arg
  	FILE *inFile = fopen(argv[ARG_INPUT], "rb");
	FILE *outFile;
	if(inFile == NULL){
		perror("Couldn't open input file.\n");

		return EXIT_FAILURE;
	}
	assert(inFile != NULL);

	if(argc >= OPT_ARGS){
		outFile = fopen(argv[ARG_OUTPUT], "w");
	}else{
		outFile = stdout;
	}

	if(outFile == NULL) {
		perror("Couldn't open output file.\n");
		return EXIT_FAILURE;
	}
	assert(outFile != NULL);

	//create an armv8 state and initialise the registers
	armv8_state *armv8 = malloc(sizeof(struct armv8_state));
	initialise(armv8);
	
	armv8->memory = malloc(MEM_SIZE);
	if(armv8->memory == NULL){
		fprintf(stderr, "Couldn't allocate buffer memory\n");
		fclose(inFile);
		return EXIT_FAILURE;
	}
	assert(armv8->memory != NULL);
	
	// Reads contents of file into buffer 
	fread(armv8->memory, 1, MEM_SIZE, inFile);	
	fclose(inFile);

	//Calls fetch function, which calls decode
	int fetch_status = fetch(armv8);
	if (fetch_status) {
		fprintf(stderr, "Couldn't execute the instruction\n");
		return EXIT_FAILURE;
	}
	assert(!fetch_status);
	
	//print armv8 state
	print_state(armv8, outFile);

	free(armv8->memory);
	free(armv8);	
	return EXIT_SUCCESS;
}
