//addressing modes
typedef enum {
	MODE_UNSIGNED_OFFSET = 0,
	MODE_REG_OFFSET,
	MODE_PRE_INDEX,
	MODE_POST_INDEX
} addressing_mode_t; 

extern int loadliteral(uint32_t instr, armv8_state *armv8); 
extern int datatransfer(uint32_t instr, armv8_state *armv8);
