// Two operand instructions 
extern uint32_t arith(char **params, int numparams); 
extern uint32_t logic(char **params, int numparams);

// Single operand with destination 
extern uint32_t wmove(char **params, int numparams); 
extern uint32_t single_op_dest(char **params, int numparams); 

// Multiply
extern uint32_t multiply(char **params, int numparams);

// Two operands, no destination 
extern uint32_t compare(char **params, int numparams); 
