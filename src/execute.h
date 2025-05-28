#include "armv8.h"
#include <stdint.h>

extern int immdp(uint32_t instr, armv8_state *armv8); 
extern int regdp(uint32_t instr, armv8_state *armv8);
extern void loadliteral(int instr, armv8_state *armv8); 
extern void datatransfer(int instr, armv8_state *armv8);
//extern void load(armv8_state *armv8, int width, int rt, uint64_t *transferAddress);
//extern void store(armv8_state *armv8, int width, int rt, uint64_t *transferAddress);
extern void branch(int instr, armv8_state *armv8);
//extern int perform_arithmetic(armv8_state *armv8, int opcode, int arg1, int arg2, int width);

