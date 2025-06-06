#include "instr-formats.h"

op0_group_t get_op0_group(unsigned int op0) {
	if ((op0 & IMMDP_MASK) == IMMDP_CODE) { return IMMDP_GROUP; }
	if ((op0 & REGDP_MASK) == REGDP_CODE) { return REGDP_GROUP; }
	if ((op0 & LDSTR_MASK) == LDSTR_CODE) { return LDSTR_GROUP; }
	if ((op0 & BR_MASK) == BR_CODE) { return BR_GROUP; }
	return OP0_INVALID;
}

const immdp_format_t immdp_format = {
	.rd    = {0, 5},
	.rn    = {5, 5},
	.imm12 = {10, 12},
	.sh    = {22, 1},
	.imm16 = {5, 16},
	.hw    = {21, 2},
	.opi   = {23, 3},
	.opc   = {29, 2},
	.sf    = {31, 1}
};

const regdp_format_t regdp_format = {
	.rd      = {0, 5},
	.rn      = {5, 5},
	.operand = {10, 6},
	.ra      = {10, 5},
	.x       = {15, 1},
	.rm      = {16, 5},
	.opr     = {21, 4},
	.N       = {21, 1},
	.shift   = {22, 2},
	.M       = {28, 1},
	.opc     = {29, 2},
	.sf      = {31, 1}
};

const sdt_format_t sdt_format = {
	.rt       = {0, 5},
	.simm19   = {5, 19},
	.xn       = {5, 5},
	.offset   = {10, 12},
	.I        = {11, 1},
	.simm9    = {12, 9},
	.R        = {21, 1},
	.xm       = {16, 5},
	.L        = {22, 1},
	.U        = {24, 1},
	.sf       = {30, 1},
	.type     = {31, 1}
};

const branch_format_t br_format = {
	.simm26 = {0, 26},
	.xn     = {5, 5},
	.cond   = {0, 4},
	.simm19 = {5, 19},
	.op     = {30, 2}
};
