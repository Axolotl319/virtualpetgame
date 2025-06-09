#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "assembly_utils.h"

void get_instr_params(char *instr, char **params, int *numparams) { 
   char *rest = NULL; 
   char *param = strtok_r(instr, " ", &rest);
   param = strtok_r(NULL, ",", &rest);  
   while (param != NULL) {
	// Remove all whitespace from the front of any parameters: 
	while (isspace(*param)) {
		param++; 
	}
	params[*numparams] = param;  
   	param = strtok_r(NULL, ",", &rest);
	(*numparams)++; 
   }
}
