//addressing modes
typedef enum {
	MODE_UNSIGNED_OFFSET,
	MODE_REG_OFFSET,
	MODE_PRE_INDEX,
	MODE_POST_INDEX
}addressing_mode_t;

extern uint32_t dt(char **params, int numparams); 

