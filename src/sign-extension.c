#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "armv8.h"
#include "sign-extension.h"
#include "execute.h"
#include "modify-regs.h"
#include <limits.h>
#include <assert.h>

int32_t sign_ext_32(int num, int msb_num) { 
	if (num >> (msb_num-1)) {
		printf("Resulting calculation: %08x\n", num | (0xffffffff << msb_num)); 
		return num | (0xffffffff << msb_num);
	}
	return num;
}

