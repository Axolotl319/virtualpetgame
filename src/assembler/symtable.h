#include <stdint.h>

typedef struct symbol_pair{
	char *label;
	uint8_t address;
} symbol_pair;

typedef struct symbol_table{
	symbol_pair **st_pairs;
	int length;
	int capacity;
} symbol_table;

//create new empty symbol table (length = 0, capacity = 1)
symbol_table *emptyST(void);

//add new label-address pair to specified symbol table
void addPair(symbol_table *st, char *label, uint8_t address);

//free space when symbol table no longer needed
void freeST(symbol_table *st);
	
//returns address of label or NULL if label is unknown
uint8_t getAddress(symbol_table *st, char *label);
