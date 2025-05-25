#ifndef ARMV8_H
#define ARMV8_H

#define NUM_GP_REGS 31 //general purpose registers

//PSTATE register
typedef struct Pstate {
	bool N; //Negative flag
	bool Z; //Zero condition flag
	bool C; //Carry condition flag
	bool V; //Overflow condition flag
} pstate;

//All the registers and memory - state of the machine
typedef struct State {
	uint8_t *memory; //Memory
	uint64_t GP_regs[NUM_GP_REGS]; //General purpose registers R0..R30
	uint64_t PC; //Program Counter
	pstate PSTATE; //PSTATE struct
} armv8_state;

#endif 

