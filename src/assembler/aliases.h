typedef int (*parse_f)(char **, int); 

typedef struct {
	char *alias;
	parse_f pf;
} alias_t;

extern parse_f lookup_alias(char *instr);
