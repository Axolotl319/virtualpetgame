//branch conditions

typedef enum branch_cond {
	BR_EQ,       //equal
	BR_NE,       //not equal
	BR_GE = 0xA, //signed greater than or equal - 0b1010
	BR_LT,       //signed less than
	BR_GT,	     //signed greater than
	BR_LE,       //signed less than or equal
	BR_AL,       //always
} branch_cond;


//branch op (bits 30/31) of branch instr
typedef enum branch_op {
	BR_OP_UNCONDITIONAL,
	BR_OP_CONDITIONAL,
	BR_INVALID, //no branch instruction corresponding to 0b10
	BR_OP_REGISTER
} branch_op;


extern int branch(uint32_t instr, armv8_state *armv8);
