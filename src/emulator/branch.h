//branch return
typedef enum branch_flag {
	BR_FAIL = -1,
	BR_NOTHING,  //no branch taken
	BR_SUCCESS
} branch_flag;

//branch op (bits 30/31) of branch instr
typedef enum branch_op {
	BR_OP_UNCONDITIONAL,
	BR_OP_CONDITIONAL,
	BR_INVALID, //no branch instruction corresponding to 0b10
	BR_OP_REGISTER
} branch_op;


extern int branch(uint32_t instr, armv8_state *armv8);
