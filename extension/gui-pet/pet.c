#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pet.h"
#include "coins.h"

//external global variable
extern int coins;

//global variable - the pet the user will interact with
pet vpet;

// const array for level information 
const level levels[MAX_LEVEL - 1] = {
	{ 1,  5},
	{ 2,  15},
	{ 3,  30},
	{ 4,  50},
	{ 5,  75},
	{ 6,  105},
	{ 7,  140},
	{ 8,  180},
	{ 9,  225},
};

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
	// fprintf(stdout, "\n"); 
	fprintf(stdout, "Stats:\n");
	check_bounds(); //ensure all stats between 0-5
	fprintf(stdout, "Level %d\n", vpet->curr_level.level_num);
	print_stat(CLEANLINESS, vpet->cleanliness);
	print_stat(HAPPINESS, vpet->happiness);
	print_stat(HUNGER, vpet->hunger);
	fprintf(stdout, "Currency: %d coins\n", coins);
}

void print_warning(int category){
	fprintf(stdout, "Warning! Only 1 heart remaining for ");
	switch(category){
		case CLEANLINESS: 
			fprintf(stdout, "cleanliness. Press C to boost!\n");
			return;

		case HAPPINESS: 
			fprintf(stdout, "happiness. Press P to boost!\n");
			return;

		case HUNGER: 
			fprintf(stdout, "hunger. Press F to boost!\n");
			return;

		default: 
			fprintf(stderr, "UNKNOWN CATEGORY\n");
			return;
	}
}

void increase_level( void ) {
	if (vpet->curr_level.level_num < MAX_LEVEL) {
		vpet->curr_level = levels[vpet->curr_level.level_num++];
		vpet->max_hearts++;
		vpet->num_actions = 0;
		vpet->level_up = false;
		fprintf(stdout, "%s has levelled up!\n", vpet->name);
	}
}


// If hunger ever reaches zero, the function is never called again. 
int dec_hunger(void *app) {
        vpet->hunger--;
        if (vpet->hunger <= MIN_HEARTS) {
                return false;
        }
        return true;
}

// If cleanliness ever reaches zero, the function is never called again 
int dec_cleanliness(void *app) {
        vpet->cleanliness--;
        if (vpet->cleanliness <= MIN_HEARTS) {
                return false;
        }
        return true;
}

// If happiness ever reaches zero, the function is never called again 
int dec_happiness(void *app) {
        vpet->happiness--;
        if (vpet->happiness <= MIN_HEARTS) {
                return false;
        }
        return true;
}

pet new_pet(char *name) {
	pet new = malloc(sizeof(struct pet));
	new->name = strdup(name);
	new->cleanliness = MAX_HEARTS;
	new->happiness   = MAX_HEARTS;
	new->hunger      = MAX_HEARTS;
	new->max_hearts  = MAX_HEARTS;
        new->curr_level  = levels[0]; 
	new->num_actions = 0; 
	new->level_up    = false; 	
	new->alive       = true;
	return new;
}

void free_pet(void) {
	free(vpet);
}
