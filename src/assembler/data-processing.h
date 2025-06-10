<<<<<<< HEAD
// Two operand instructions 
extern int arith(char **params, int numparams); 
extern int logic(char **params, int numparams);

// Single operand with destination 
extern int wmove(char **params, int numparams); 
extern int single_op_dest(char **params, int numparams); 

// Multiply
extern int multiply(char **params, int numparams);

// Two operands, no destination 
extern int compare(char **params, int numparams); 
