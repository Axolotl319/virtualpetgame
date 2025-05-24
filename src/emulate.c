#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

//print the armv8 state - need to modify to print to out file if specified
void print_state(armv8_state *armv8) {
	//General Purpose Registers
	printf("Registers:\n");
	for (int i = 0; i < NUM_GP_REGS; i++) {
		printf("X%02d = %016lx\n", i, armv8->GP_regs[i]);
	}

	//PC
	printf("PC  = %016lx\n", armv8->PC);

	//PSTATE
	printf("PSTATE: %c%c%c%c\n", 
			armv8->PSTATE.N ? 'N' : '-',
			armv8->PSTATE.Z ? 'Z' : '-',
			armv8->PSTATE.C ? 'C' : '-',
			armv8->PSTATE.V ? 'V' : '-');

	//Memory
	printf("Non-zero memory:\n");
	//have not implemented memory yet

}	


int main(int argc, char **argv) {
	//create an armv8 state and initialise the registers
	armv8_state armv8;
	initialise(&armv8);

	//print armv8 state
	print_state(&armv8);


  return EXIT_SUCCESS;
}
