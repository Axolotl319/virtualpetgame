typedef struct {
	char * cond_str;
	branch_cond bcond;
} cond_pair;


extern int reg_branch( char ** params, int numparams, uint32_t * instr );
extern int uncond_branch( char ** params, int numparams, uint32_t * instr );
extern int cond_branch( char ** params, int numparams, uint32_t * instr );
