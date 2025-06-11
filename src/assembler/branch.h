typedef struct {
	char * cond_str;
	branch_cond bcond;
} cond_pair;


extern uint32_t reg_branch( char ** params, int numparams );
extern uint32_t uncond_branch( char ** params, int numparams );
extern uint32_t cond_branch( char ** params, int numparams ); 
