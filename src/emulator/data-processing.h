//regdp instr type
typedef enum regdp_instr_t {
	ARITH_INSTR,
	LOG_INSTR,
	MUL_INSTR,
	REGDP_INVALID
} regdp_instr_t;


extern int immdp(uint32_t instr, armv8_state *armv8); 
extern int regdp(uint32_t instr, armv8_state *armv8);
