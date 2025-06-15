#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pet.h"

//#define MAX/MIN as 5/0, not sure if we want an overall too?

void check_bounds(pet p){
	if(p->cleanliness > 5){
		p->cleanliness = 5;
		printf("debug: cleanliness set to > 5, reset to 5");
	}else if(p->cleanliness < 0){
		p->cleanliness = 0;
		printf("debug: cleanliness set to < 0, reset to 0");
	}

	if(p->happiness > 5){
		p->happiness = 5;
		printf("debug: happiness set to > 5, reset to 5");
	}else if(p->happiness < 0){
		p->happiness = 0;
		printf("debug: happiness set to < 0, reset to 0");
	}

	if(p->hunger > 5){
		p->hunger = 5;
		printf("debug: hunger set to > 5, reset to 5");
	}else if(p->hunger < 0){
		p->hunger = 0;
		printf("debug: hunger set to < 0, reset to 0");
	}
}

//probs use enum
//for now category = 0/1/2
static void print_stat(int category, int amt){
	switch(category){
		case 0: fprintf(stdout, "Cleaniness ");
			break;

		case 1: fprintf(stdout, "Happiness ");
			break;

		case 2: fprintf(stdout, "Hunger ");
			break;

		default: fprintf(stderr, "Unknown category");
			 return;
			 break;
	}
	for(int i = 0; i < amt; i++){
		fprintf(stdout, "+");
	}
	fprintf(stdout, "\n");
}

void print_hearts(pet p) {
	fprintf(stdout, "Stats:\n");
	check_bounds(p); //ensure all stats between 0-5
	print_stat(0, p->cleanliness);
	print_stat(1, p->happiness);
	print_stat(2, p->hunger);
}

pet new_pet(char *name) {
	pet new = malloc(sizeof(struct pet));
	new->name = strdup(name);
	new->cleanliness = 5;
	new->happiness = 5;
	new->hunger = 5;
	return new;
}

void free_pet(pet p) {
	free(p);
}
