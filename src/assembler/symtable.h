#include <stdint.h>

typedef struct symbol_table{
	char **labels; //array of strings, size decided in first pass
	uint8_t *addresses; //associated address
	//labels[i] has address addresses[i]
} symbol_table;

//returns NULL if error, else returns pointer to new symbol table
//elements = number of (label, address) elements
extern symbol_table *createST(char **labels, uint8_t *addresses, int elements);

//frees space previously mallocd to a symbol table st
extern void freeST(symbol_table *st);
