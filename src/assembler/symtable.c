#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* So what I'm thinking is:
 * 1st pass, the labels and addresses arrays are created
 * Then a symbol table is made which does not change
 */

struct symbol_table{
	char **labels; //array of strings, size decided in first pass
	uint8_t *addresses; //associated address
	//labels[i] has address addresses[i]
}

typedef struct symbol_table *symbol_table; //ADT

//returns NULL if error, else returns pointer to new symbol table
//elements = number of (label, address) elements
symbol_table createST(char **labels, uint8_t *addresses, int elements){
	symbol_table table = malloc(sizeof(struct symbol_table));
	if(table == NULL){
		fprintf(stderr, "Can't allocate memory for new symbol table");
		return NULL;
	}
	table->labels = malloc(elements * sizeof(char*));
	if(table->labels == NULL){
		fprintf(stderr, "Can't allocate memory for symbol table labels array");
		return NULL;
	}
	table->addresses = malloc(elements * sizeof(uint8_t));
	if(table->addresses == NULL){
		fprintf(stderr, "Can't allocate memory for ST addresses array");
		return NULL;
	}
	for(int i = 0; i < elements; i++){
		strcpy(table->*labels[i], *(labels + i)); 
		//have included both array syntaxes for now, not sure if only one
		//works or is preferred
		table->*addresses[i] = *(addresses + i);
	}
	return table; //pointer to symbol table
}

//free space when symbol table no longer needed
void freeST(symbol_table st){
	free(st->labels);
	free(st->addresses);
	free(st);
}
