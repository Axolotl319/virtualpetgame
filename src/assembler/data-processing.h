// Two operand instructions
extern int arith( char ** params, int numparams, uint32_t * toReturn );
extern int logic( char ** params, int numparams, uint32_t * toReturn );

//Single operand with destination
extern int wmove( char ** params, int numparams, uint32_t * toReturn );
extern int single_op_dest( char ** params, int numparams, uint32_t * toReturn );

//Multiply
extern int multiply( char ** params, int numparams, uint32_t * toReturn );

//Two operands, no destination
extern int compare( char ** params, int numparams, uint32_t * toReturn );
