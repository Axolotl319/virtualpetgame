//immdp instructions
#define OPI_ARITH 0x2 //opi = 0b010 then immdp arithmetic instr
#define OPI_MOV   0x5 //opi = 0b101 then immdp wide move
//regdp instructions
#define ARITH_MASK 0x19 //arith instrs = 01xx0
#define ARITH_CODE 0x8
#define LOG_MASK 0x18 //log instrs = 00xxx
#define LOG_CODE 0x0
#define MUL_CODE 0x18 //mul instrs = 11000
//possible shifts for arith or move instructions
#define ARITH_SHIFT_AMT 12
#define MOV_SHIFT_AMT 16

//regdp instr type
typedef enum regdp_instr_t {
	ARITH_INSTR,
	LOG_INSTR,
	MUL_INSTR,
	REGDP_INVALID
} regdp_instr_t;


extern int immdp(uint32_t instr, armv8_state *armv8); 
extern int regdp(uint32_t instr, armv8_state *armv8);
