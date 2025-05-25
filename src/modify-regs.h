#include "armv8.h"

extern int check_reg_bounds( int reg_num );
extern int read_32( armv8_state * armv8, int reg_num, uint32_t * data );
extern int write_32( armv8_state * armv8, int reg_num, uint32_t data );
extern int read_64( armv8_state * armv8, int reg_num, uint64_t * data );
extern int write_64( armv8_state * armv8, int reg_num, uint64_t data );
extern int incrementPC( armv8_state * armv8 );
extern int modifyPC( armv8_state * armv8, uint64_t addr );
