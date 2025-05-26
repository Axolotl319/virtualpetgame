#include "armv8.h"

void immdp(int instr); 
void regdp(int instr);
void loadliteral(int instr); 
void datatransfer(int instr);
void branch(int instr, armv8_state *armv8);
