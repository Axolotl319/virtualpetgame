#include <stdint.h>
#include "sign-extension.h"

int32_t sign_ext_32(int num, int msb_num) { 
	if (num >> (msb_num-1)) { 
		return num | (0xffffffff << msb_num);
	}
	return num;
}

