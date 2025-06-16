typedef int (*parse_f)(char **, int, uint32_t *tobin);

//n is used for logic instrs
typedef struct {
	const char *instr;
	int code;
	int n;
} code_map;

typedef struct {
	char *instr;
	char *alias;
	parse_f pf;
	int insert_xzr;
} func_map;

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
