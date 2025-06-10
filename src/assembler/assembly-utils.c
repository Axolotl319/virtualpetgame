#include <stdint.h>
#include <stdlib.h>
#include "assembly-utils.h"

void update_sf(uint32_t *toReturn, int bitnum, char *param) {
        if (*param == 'x') {
           *toReturn |= (1 << bitnum);
        }
}

uint8_t obtain_reg_num(char *param) {
        return atoi(++param);
}
