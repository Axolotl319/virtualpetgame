#include "armv8.h"
enum operation {
	OP_ADD,
	OP_SUB,
	OP_LOGIC
};

extern int read_reg32( armv8_state * armv8, int reg_num, uint32_t * data );
extern int write_reg32( armv8_state * armv8, int reg_num, uint64_t data );
extern int read_reg64( armv8_state * armv8, int reg_num, uint64_t * data );
extern int write_reg64( armv8_state * armv8, int reg_num, uint64_t data );
extern int incrementPC( armv8_state * armv8 );
extern int setPC( armv8_state * armv8, uint64_t addr );
extern void update_pstate(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result, enum operation op_type, int width);
extern uint64_t logical_shift_left(armv8_state *armv8, int reg_num, int shift, int width);
extern uint64_t logical_shift_right(armv8_state *armv8, int reg_num, int shift, int width);
extern uint64_t arithmetic_shift_right(armv8_state *armv8, int reg_num, int shift, int width);
extern uint64_t rotate_right(armv8_state *armv8, int reg_num, int shift, int width);
