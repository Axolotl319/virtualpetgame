#include <stdint.h>

struct symbol_pair{
	char *label;
	uint32_t address;
};

typedef struct symbol_pair *symbol_pair;

struct symbol_table{
	symbol_pair *st_pairs;
	int length;
	int capacity;
};

typedef struct symbol_table *symbol_table;

//create new empty symbol table (length = 0, capacity = 1)
symbol_table emptyST(void);

//add new label-address pair to specified symbol table
int addPair(symbol_table st, char *label, uint32_t address);

//free space when symbol table no longer needed
void freeST(symbol_table st);
	
//returns address of label or NULL if label is unknown
uint32_t getAddress(symbol_table st, char *label);
