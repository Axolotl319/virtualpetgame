#define NUM_GP_REGS 31     //num GP registers
#define MEM_SIZE (1 << 21) //ARMV8 has 2MB memory
#define ZRSP 0x1f          // Zero Register/Stack Pointer

//PSTATE register
typedef struct pstate {
	bool N; //Negative flag
	bool Z; //Zero condition flag
	bool C; //Carry condition flag
	bool V; //Overflow condition flag
} pstate;

//All the registers and memory - state of the machine
typedef struct armv8_state {
	uint64_t GP_regs[NUM_GP_REGS]; //General purpose registers R0..R30
	uint64_t PC; //Program Counter
	pstate PSTATE; //PSTATE struct
	uint8_t *memory; //Memory
} armv8_state;

