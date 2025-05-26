#include "armv8.h"
extern int read_reg32( armv8_state * armv8, int reg_num, uint32_t * data );
extern int write_reg32( armv8_state * armv8, int reg_num, uint32_t data );
extern int read_reg64( armv8_state * armv8, int reg_num, uint64_t * data );
extern int write_reg64( armv8_state * armv8, int reg_num, uint64_t data );
extern int incrementPC( armv8_state * armv8 );
extern int setPC( armv8_state * armv8, uint64_t addr );
extern void update_pstate_add32( pstate * PSTATE, uint32_t op1, uint32_t op2, uint32_t result );
extern void update_pstate_add64( pstate * PSTATE, uint64_t op1, uint64_t op2, uint64_t result );
extern void update_pstate_sub32( pstate * PSTATE, uint32_t op1, uint32_t op2, uint32_t result );
extern void update_pstate_sub64( pstate * PSTATE, uint64_t op1, uint64_t op2, uint64_t result );
extern void update_pstate_logic32( pstate * PSTATE, uint32_t result );
extern void update_pstate_logic64( pstate * PSTATE, uint64_t result );
extern void logical_shift_left(armv8_state *armv8, int reg_num, int width);
extern void logical_shift_right(armv8_state *armv8, int reg_num, int width);
extern void arithmetic_shift_left(armv8_state *armv8, int reg_num, int width);
extern void rotate_right(armv8_state *armv8, int reg_num, int width);
