typedef struct {
	char *alias;
	char *base;
} alias_t;

extern char *lookup_alias(char *instr);
