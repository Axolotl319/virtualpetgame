#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "assembly-utils.h"

/*
 Given the instruction to update, the bitnum at which sf exists, and a random 
 register parameter, updates the sf bit in toReturn based on width specified 
 by param. 
 */
void update_sf(uint32_t *toReturn, int bitnum, char *param) {
	if (*param == 'x') {
           *toReturn |= (1 << bitnum);
        }
}

/*
 Given the full register name (for example, "x8"), returns the integer 
 corresponding to the register number (for example, the integer "8").

 Returns 0x1f for zero registers, as needed.  
 */
uint8_t obtain_reg_num(char *param) {
	if (!strcmp(param, "xzr") || !strcmp(param, "wzr")) {
		return 0x1f; 
	}
        return atoi(++param);
}

/*
 Given the full immediate parameter (for example, "#0x1"), returns the 
 integer corresponding to the immediate value (for example, the integer "1"). 
 */
uint32_t extract_imm(char *param) {
	if (strchr(param, 'x') == NULL) {
		return atoi(++param); 
	}
	return strtol(++param, NULL, 16); 	
}

uint8_t obtain_shift_amt(char *param) {
        char shifttype[5];
        char shiftamt[5];
        sscanf(param, "%s %s", shifttype, shiftamt);
        return extract_imm(shiftamt);
}
