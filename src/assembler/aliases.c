#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aliases.h"

#define NUM_ALIASES 28

//alphabetically sorted alias table
//This doesn't deal with b.cond because im not sure what to do for that
static alias_t alias_table[] = {
	{ "add" , "add"  },
	{ "adds", "adds" },
	{ "and" , "and"  },
	{ "ands", "ands" },
	{ "b"   , "b"    },
	{ "bic" , "bic"  },
	{ "bics", "bics" },
	{ "br"  , "br"   },
	{ "cmn" , "adds" },
	{ "cmp" , "subs" },
	{ "eon" , "eon"  },
	{ "eor" , "eor"  },
	{ "ldr" , "ldr"  },
	{ "madd", "madd" },
	{ "mneg", "msub" },
	{ "mov" , "orr"  },
	{ "movk", "movk" },
	{ "movn", "movn" },
	{ "movz", "movz" },
	{ "msub", "msub" },
	{ "mul" , "madd" },
	{ "mvn" , "orn"  },
	{ "neg" , "sub"  },
	{ "negs", "subs" },
	{ "orn" , "orn"  },
	{ "orr" , "orr"  },
	{ "str" , "str"  },
	{ "tst" , "ands" },
};

static int cmp_alias(const void *key, const void *elem) {
    const char *alias_key = (const char *)key;
    const alias_t *alias_elem = (const alias_t *)elem;
    return strcmp(alias_key, alias_elem->alias);
}

//lookup function
//returns NULL if instruction not found
char* lookup_alias(char *instr) {
	alias_t *res = bsearch(instr, alias_table, NUM_ALIASES, sizeof(alias_table[0]), &cmp_alias);

	if (res == NULL) { return NULL; }

	return res->base;
}

