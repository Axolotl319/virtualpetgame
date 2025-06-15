// Two operand instructions
extern int arith( char ** params, int numparams, uint32_t * instr );
extern int logic( char ** params, int numparams, uint32_t * instr );

//Single operand with destination
extern int wmove( char ** params, int numparams, uint32_t * instr );
extern int single_op_dest( char ** params, int numparams, uint32_t * instr );

//Multiply
extern int multiply( char ** params, int numparams, uint32_t * instr );

//Two operands, no destination
extern int compare( char ** params, int numparams, uint32_t * instr );
