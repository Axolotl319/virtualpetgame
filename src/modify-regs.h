enum operation {
	OP_ADD,
	OP_SUB,
	OP_LOGIC
};

extern int write_reg( armv8_state * armv8, int reg_num, int64_t data, int width);
extern int read_reg32( armv8_state * armv8, int reg_num, int32_t * data );
extern int read_reg64( armv8_state * armv8, int reg_num, int64_t * data );
extern void incrementPC( armv8_state * armv8 );
extern int setPC( armv8_state * armv8, uint64_t addr );
extern void update_pstate(pstate *PSTATE, uint64_t op1, uint64_t op2, uint64_t result, enum operation op_type, int width);

