#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "execute.h"

const int WORD_SIZE = 4; 	// 1 word = 4 bytes
const int MEM_SIZE = 1 << 21; 	// ARMv8 has 2MB memory 
#define NUM_GP_REGS 31 //number of general purpose registers

//PSTATE register
typedef struct Pstate {
	bool N; //Negative flag
	bool Z; //Zero condition flag
	bool C; //Carry condition flag
	bool V; //Overflow condition flag
} pstate;

//All the registers - state of the machine
typedef struct Registers {
	uint64_t GP_regs[NUM_GP_REGS]; //General purpose registers R0..R30
	uint64_t PC; //Program Counter
	pstate PSTATE; //PSTATE struct
} armv8_state;

//initialise the registers to 0. Set PSTATE Z flag to 1.
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
	//have not implemented memory yet

}
// Given the starting address of an instruction, determines the instruction type 
// and passes the instruction to the corresponding function to handle. 
// Returns 1 if decoding unsuccessful, returns 0 if successful.  
int decode(char *instruction) {
	
	// Combines four consecutive bytes to 32 bits, taking into account little endian  
	int result = ((0xff & *(instruction+3)) << 24) 
		| ((0xff & *(instruction+2)) << 16) 
		| ((0xff & *(instruction+1)) << 8) 
		| ((0xff & *instruction));

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
			perror("Bad opcode (op0).\n");
			return 1;
			break; 
	}

	return 0; 
	
	/*
	if (opzero == 8 || opzero == 9) {
		printf("This is data processing (immediate).\n");
		immdp( result ); 
	} else if (opzero == 5 || opzero == 13) {
		printf("This is data processing (registers).\n");
		regdp( result ); 
	} else if (opzero == 4 || opzero == 6 || opzero == 12 || opzero == 14) {
		printf("This is single data transfer.\n"); 
		int temp = 0xf & (result >> 31); 
		if (temp) {
			datatransfer( result ); 
		} else {
			loadliteral( result ); 
		}
	} else if (opzero == 10 || opzero == 11) {
		printf("This is branch.\n"); 
		branch( result ); 
	} else {
		perror("Bad opcode (op0).\n");
		return 1; 
	}
	
	return 0;
       	*/	
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

	char *buffer; // 1 char = 1 byte = 8 bits
	buffer = malloc(MEM_SIZE);
	if(buffer == NULL){
		perror("Couldn't allocate buffer memory");
		fclose(inFile);
		return 1;
	}

	// Obtains size of file, kept in case needed later 
	// fseek(inFile, 0, SEEK_END); 
	// int filesize = ftell(inFile); 
	// rewind(inFile);
	
	// Reads contents of file into buffer 
	fgets(buffer, MEM_SIZE, inFile);	
	fclose(inFile);
		
	// Decodes each instruction 
	char *current = buffer; 
	while( (*(current+WORD_SIZE-1) & 0xff) != 0x8a ) {
		if ( decode( current ) ) {
			return 1; 
		}	
		current += WORD_SIZE; 
	}	
	
	free(buffer);

	//print armv8 state
	print_state(&armv8, outFile);

	return EXIT_SUCCESS;
}
