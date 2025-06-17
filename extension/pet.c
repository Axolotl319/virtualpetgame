#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pet.h"

//global variable - the pet the user will interact with
pet vpet;

void check_bounds(void){
	if(vpet->cleanliness > MAX_HEARTS){
		vpet->cleanliness = MAX_HEARTS;
		printf("debug: cleanliness set to > 5, reset to 5\n");
	}else if(vpet->cleanliness < MIN_HEARTS){
		vpet->cleanliness = MIN_HEARTS;
		printf("debug: cleanliness set to < 0, reset to 0\n");
	}

	if(vpet->happiness > MAX_HEARTS){
		vpet->happiness = MAX_HEARTS;
		printf("debug: happiness set to > 5, reset to 5\n");
	}else if(vpet->happiness < MIN_HEARTS){
		vpet->happiness = MIN_HEARTS;
		printf("debug: happiness set to < 0, reset to 0\n");
	}

	if(vpet->hunger > MAX_HEARTS){
		vpet->hunger = MAX_HEARTS;
		printf("debug: hunger set to > 5, reset to 5\n");
	}else if(vpet->hunger < MIN_HEARTS){
		vpet->hunger = MIN_HEARTS;
		printf("debug: hunger set to < 0, reset to 0\n");
	}
}

static void print_stat(int category, int amt){
	switch(category){
		case CLEANLINESS: 
			fprintf(stdout, "Cleaniness ");
			break;

		case HAPPINESS: 
			fprintf(stdout, "Happiness ");
			break;

		case HUNGER: 
			fprintf(stdout, "Hunger ");
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
	print_stat(CLEANLINESS, vpet->cleanliness);
	print_stat(HAPPINESS, vpet->happiness);
	print_stat(HUNGER, vpet->hunger);
}

void print_warning(int category){
	fprintf(stdout, "Warning! Only 1 heart remaining for ");
	switch(category){
		case CLEANLINESS: 
			fprintf(stdout, "cleanliness. Press C to boost!");
			return;

		case HAPPINESS: 
			fprintf(stdout, "happiness. Press P to boost!");
			return;

		case HUNGER: 
			fprintf(stdout, "hunger. Press F to boost!");
			return;

		default: fprintf(stderr, "UNKNOWN CATEGORY");
			 return;
	}
}

pet new_pet(char *name) {
	pet new = malloc(sizeof(struct pet));
	new->name = strdup(name);
	new->cleanliness = MAX_HEARTS;
	new->happiness   = MAX_HEARTS;
	new->hunger      = MAX_HEARTS;
	new->alive       = true;
	return new;
}

void free_pet(void) {
	free(vpet);
}
