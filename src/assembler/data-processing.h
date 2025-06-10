// Two operand instructions 
extern int arith(char *instr); 
extern int logic(char *instr);

// Single operand with destination 
extern int wmove(char *instr); 
extern int single_op_dest(char *instr); 

// Multiply
extern int multiply(char *instr);

// Two operands, no destination 
extern int compare(char *instr); 
