#include <stdio.h>
#include <stdlib.h>
#include "pet.h"

//#define MAX/MIN as 5/0, not sure if we want an overall too?

void check_bounds(hearts h){
	if(h->cleanliness > 5){
		h->cleanliness = 5;
		printf("debug: cleanliness set to > 5, reset to 5");
	}else if(h->cleanliness < 0){
		h->cleanliness = 0;
		printf("debug: cleanliness set to < 0, reset to 0");
	}

	if(h->happiness > 5){
		h->happiness = 5;
		printf("debug: happiness set to > 5, reset to 5");
	}else if(h->happiness < 0){
		h->happiness = 0;
		printf("debug: happiness set to < 0, reset to 0");
	}

	if(h->hunger > 5){
		h->hunger = 5;
		printf("debug: hunger set to > 5, reset to 5");
	}else if(h->hunger < 0){
		h->hunger = 0;
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

void print_hearts(hearts h){
	fprintf(stdout, "Stats:\n");
	check_bounds(h); //ensure all stats between 0-5
	print_stat(0, h->cleanliness);
	print_stat(1, h->happiness);
	print_stat(2, h->hunger);
}

static hearts new_pet(void){
	hearts new = malloc(sizeof(struct hearts));
	new->cleanliness = 5;
	new->happiness = 5;
	new->hunger = 5;
	return new;
}

static void free_hearts(hearts h){
	free(h);
}

int main(void){
	hearts pet = new_pet();
	return EXIT_SUCCESS;
}
