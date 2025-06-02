#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/* So what I'm thinking is:
 * 1st pass, the labels and addresses arrays are created
 * Then a symbol table is made which does not change
 */

//returns NULL if error, else returns pointer to new symbol table
//elements = number of (label, address) elements
symbol_table *createST(char **labels, uint8_t *addresses, int elements){
	symbol_table *table = malloc(sizeof(struct symbol_table));
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
		table->labels[i] = labels[i]; 
		table->addresses[i] = addresses[i];
	}
	return table; //pointer to symbol table
}

//free space when symbol table no longer needed
void freeST(symbol_table *st){
	free(st->labels);
	free(st->addresses);
	free(st);
}
