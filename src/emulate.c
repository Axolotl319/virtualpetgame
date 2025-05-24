#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define WORD_SIZE 4; //1 word = 4 bytes
		     
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
 
void decode(char *instruction);

int main(int argc, char **argv) 
{
	//create an armv8 state and initialise the registers
	armv8_state armv8;
	initialise(&armv8);

	//argc = 2 or 3, ./emulate is 1st arg
  	FILE *inFile = fopen(argv[1], "r");
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

	int MEM_SIZE = 1 << 18;
	char *buffer; //1 char = 1 byte = 8 bits
	buffer = malloc(MEM_SIZE);
	if(buffer == NULL){
		perror("Couldn't allocate buffer memory");
		fclose(inFile);
		return 1;
	}

	fgets(buffer, MEM_SIZE, inFile);
	fclose(inFile);

	char *current = buffer;
	while(*current != 0){
		decode(current); //function to be written
		current += WORD_SIZE; //next instruction
	}
	free(buffer);

	//print armv8 state
	print_state(&armv8, outFile);

	return EXIT_SUCCESS;
}
