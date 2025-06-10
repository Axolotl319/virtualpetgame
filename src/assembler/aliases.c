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
	{ "add" , &dp },
	{ "adds", &dp },
	{ "and" , &dp },
	{ "ands", &dp },
	{ "b"   , &branch },
	{ "b."  , &branch }, 
	{ "bic" , &branch },
	{ "bics", &branch },
	{ "br"  , &branch },
	{ "cmn" , &dp },
	{ "cmp" , &dp },
	{ "eon" , &dp },
	{ "eor" , &dp },
	{ "ldr" , &dt },
	{ "madd", &dp },
	{ "mneg", &dp },
	{ "mov" , &dp },
	{ "movk", &dp },
	{ "movn", &dp },
	{ "movz", &dp },
	{ "msub", &dp },
	{ "mul" , &dp },
	{ "mvn" , &dp },
	{ "neg" , &dp },
	{ "negs", &dp },
	{ "orn" , &dp },
	{ "orr" , &dp },
	{ "str" , &dt },
	{ "sub" , &dp }, 
	{ "subs", &dp },
	{ "tst" , &dp },
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

