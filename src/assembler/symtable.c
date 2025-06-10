#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtable.h"

/* 1st pass, the labels and addresses arrays are created
 * Then a symbol table is made which does not change
 */

//returns NULL if error, else returns pointer to new symbol table
symbol_table emptyST(void){
	symbol_table st = malloc(sizeof(struct symbol_table));
	if(st == NULL){
		fprintf(stderr, "Can't allocate memory for new symbol table\n");
		return NULL;
	}
	st->length = 0; //contains no elements
	st->capacity = 1; //capacity > length
	st->st_pairs = malloc(st->capacity * sizeof(symbol_pair));
	return st;
}

int addPair(symbol_table st, char *label, uint8_t address){
	symbol_pair np = malloc(sizeof(struct symbol_pair)); //np = new pair
	if(np == NULL){
		fprintf(stderr, "Can't allocate memory for new symbol table pair\n");
		return 1;
	}
	np->label = strdup(label);
	if (np->label == NULL) { 
		fprintf(stderr, "Cannot allocate memory for label\n");
		free(np);
		return 1; 
	}

	np->address = address;

	if(st->length >= st->capacity){ //current array too small to add pair
		int new_capacity = st->capacity * 2; //double the array capacity
		symbol_pair *tmp = realloc(st->st_pairs, st->capacity * sizeof(symbol_pair));
		if(tmp == NULL){
			fprintf(stderr, "Can't reallocate memory for symbol table pairs\n");
			free(np->label);
			free(np);
			return 1;
		}
		
		st->st_pairs = tmp;
		st->capacity = new_capacity;
	}

	st->st_pairs[st->length++] = np;
	assert(st->capacity >= st->length);
	return 0;
}

//free space when symbol table no longer needed
void freeST(symbol_table st){
	for(int i = 0; i < st->length; i++){
		free(st->st_pairs[i]);
	}
	free(st->st_pairs);
	free(st);
}

//get address. returns 1 on failure
uint8_t getAddress(symbol_table st, char *label){
	for(int i = 0; i < st->length; i++){
		symbol_pair p = st->st_pairs[i];
		if(!strcmp(p->label, label)){
			return p->address;
		}
	}
	fprintf(stderr, "Label is not in symbol table so can't get address\n");
	return 1; //address has to be a multiple of 4 so can never be 1
}

