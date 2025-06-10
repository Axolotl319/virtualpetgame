#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data-processing.h"
#include "data-transfer.h"
#include "branch.h"
#include "aliases.h"

#define NUM_ALIASES 31

//alphabetically sorted alias table
//This doesn't deal with b.cond because im not sure what to do for that
static alias_t alias_table[] = {
	{ "add" , &arith },
	{ "adds", &arith },
	{ "and" , &logic },
	{ "ands", &logic },
	{ "b"   , &branch },
	{ "b."  , &branch }, 
	{ "bic" , &branch },
	{ "bics", &branch },
	{ "br"  , &branch },
	{ "cmn" , &compare },
	{ "cmp" , &compare },
	{ "eon" , &logic },
	{ "eor" , &logic },
	{ "ldr" , &dt },
	{ "madd", &multiply },
	{ "mneg", &single_op_dest },
	{ "mov" , &single_op_dest },
	{ "movk", &wmove },
	{ "movn", &wmove },
	{ "movz", &wmove },
	{ "msub", &multiply },
	{ "mul" , &single_op_dest },
	{ "mvn" , &single_op_dest },
	{ "neg" , &single_op_dest },
	{ "negs", &single_op_dest },
	{ "orn" , &logic },
	{ "orr" , &logic },
	{ "str" , &dt },
	{ "sub" , &arith }, 
	{ "subs", &arith },
	{ "tst" , &compare },
};

static int cmp_alias(const void *key, const void *elem) {
    const char *alias_key = (const char *)key;
    const alias_t *alias_elem = (const alias_t *)elem;
    return strcmp(alias_key, alias_elem->alias);
}

//lookup function
//returns NULL if instruction not found
parse_f lookup_alias(char *instr) {
	alias_t *res = bsearch(instr, alias_table, NUM_ALIASES, sizeof(alias_table[0]), &cmp_alias);

	if (res == NULL) { return NULL; }

	return res->pf;
}

