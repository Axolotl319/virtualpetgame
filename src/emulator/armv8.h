#include "instr-formats.h"

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

extern int write_reg( armv8_state * armv8, int reg_num, uint64_t data, int width );
extern int read_reg( armv8_state * armv8, int reg_num, uint64_t * data, int width );
extern void incrementPC( armv8_state * armv8 );
extern int setPC( armv8_state * armv8, uint64_t addr );
extern void update_pstate( pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result, operation op_type, int width);
extern int get_memory_data( armv8_state * armv8, uint64_t addr, int num_bytes, uint64_t * data );	
