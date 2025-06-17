#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pet.h"

//global variable - the pet the user will interact with
pet vpet;

//#define MAX/MIN as 5/0, not sure if we want an overall too?

void check_bounds(void){
	if(vpet->cleanliness > 5){
		vpet->cleanliness = 5;
		printf("debug: cleanliness set to > 5, reset to 5\n");
	}else if(vpet->cleanliness < 0){
		vpet->cleanliness = 0;
		printf("debug: cleanliness set to < 0, reset to 0\n");
	}

	if(vpet->happiness > 5){
		vpet->happiness = 5;
		printf("debug: happiness set to > 5, reset to 5\n");
	}else if(vpet->happiness < 0){
		vpet->happiness = 0;
		printf("debug: happiness set to < 0, reset to 0\n");
	}

	if(vpet->hunger > 5){
		vpet->hunger = 5;
		printf("debug: hunger set to > 5, reset to 5\n");
	}else if(vpet->hunger < 0){
		vpet->hunger = 0;
		printf("debug: hunger set to < 0, reset to 0\n");
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

		default: fprintf(stderr, "Unknown category\n");
			 return;
			 break;
	}
	for(int i = 0; i < amt; i++){
		fprintf(stdout, "+");
	}
	fprintf(stdout, "\n");
}

void print_hearts(void) {
	fprintf(stdout, "Stats:\n");
	check_bounds(); //ensure all stats between 0-5
	print_stat(0, vpet->cleanliness);
	print_stat(1, vpet->happiness);
	print_stat(2, vpet->hunger);
}

void print_warning(int category){
	fprintf(stdout, "Warning! Only 1 heart remaining for ");
	switch(category){
		case 0: fprintf(stdout, "cleanliness. Press C to boost!");
			return;

		case 1: fprintf(stdout, "happiness. Press P to boost!");
			return;

		case 2: fprintf(stdout, "hunger. Press F to boost!");
			return;

		default: fprintf(stderr, "UNKNOWN CATEGORY");
			 return;
	}
}

pet new_pet(char *name) {
	pet new = malloc(sizeof(struct pet));
	new->name = strdup(name);
	new->cleanliness = 5;
	new->happiness   = 5;
	new->hunger      = 5;
	new->alive       = 1;
	return new;
}

void free_pet(void) {
	free(vpet);
}
