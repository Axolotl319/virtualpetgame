#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "DEV_Config.h"
#include "LCD_1in3.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "pet.h"
#include "coins.h"

//Helper macro to convert hours to seconds
#define HOURS(x) ((x) * 3600)

//locks for incrementing/decrementing stats
pthread_mutex_t cleanliness_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t happiness_mutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t hunger_mutex      = PTHREAD_MUTEX_INITIALIZER;

//external global variable
extern int coins;

//global variable - the pet the user will interact with
pet vpet;

//const array for level information
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

//checks whether the hearts are within range
void check_bounds(int category){
	switch(category) {
		case CLEANLINESS:
			if(vpet->cleanliness > vpet->max_hearts){
				vpet->cleanliness = vpet->max_hearts;
			}else if(vpet->cleanliness < 0){
				vpet->cleanliness = 0;
			}

		case HAPPINESS:
			if(vpet->happiness > vpet->max_hearts){
				vpet->happiness = vpet->max_hearts;
			}else if(vpet->happiness < 0){
				vpet->happiness = 0;
			}

		case HUNGER:
			if(vpet->hunger > vpet->max_hearts){
				vpet->hunger = vpet->max_hearts;
			}else if(vpet->hunger < 0){
				vpet->hunger = 0;
			}
	}
}

//returns a string with the number of hearts in a stat
//takes in the stat category, the number of hearts,
//and a pointer to a string, which stores the final string
void get_stat_string(int category, int amt, char *stat_str) {

	snprintf(stat_str, MAX_STAT_LEN, "%s", "");
	size_t len = strlen(stat_str);
	size_t amt_to_append = (MAX_STAT_LEN - len - 1) < amt ?
			       (MAX_STAT_LEN - len - 1) : amt;

        for(int i = 0; i < amt_to_append; i++){
                stat_str[len + i] = '+';
        }
        
	stat_str[len + amt_to_append] = '\0';

}

//prints warnings if there is only 1 heart in any stat
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

		default: fprintf(stderr, "UNKNOWN CATEGORY\n");
			 return;
	}
}

//levels up the pet
void increase_level( void ) {
	if (vpet->curr_level.level_num < MAX_LEVEL) {
		vpet->curr_level = levels[vpet->curr_level.level_num++];
		vpet->max_hearts++;
		vpet->num_actions = 0;
		vpet->level_up = false;
		fprintf(stdout, "%s has levelled up!\n", vpet->name);
	}
}

//initialiser
pet new_pet(char *name) {
	pet new = malloc(sizeof(struct pet));
	new->name = strdup(name);
	new->cleanliness = INIT_HEARTS;
	new->happiness   = INIT_HEARTS;
	new->hunger      = INIT_HEARTS;
	new->max_hearts  = INIT_HEARTS;
	new->curr_level  = levels[0];	
	new->num_actions = 0;
	new->level_up    = false;
	new->alive       = true;
	return new;
}

void free_pet(void) {
	free(vpet);
}

//Cleanliness droppine one bar every 12 hours
void *decrease_cleanliness(void *arg) {
    bool *running = (bool *)arg;
    while (*running) {
        sleep(HOURS(CLEANLINESS_DECAY_HOURS));
	//lock before decrementing
	pthread_mutex_lock(&cleanliness_mutex);
	vpet->cleanliness--;
        check_bounds(CLEANLINESS);
	pthread_mutex_unlock(&cleanliness_mutex);
    }
    return NULL;
}

//Hunger dropping one bar every 5 hours
void *decrease_hunger(void *arg) {
    bool *running = (bool *)arg;
    while (*running) {
        sleep(HOURS(HUNGER_DECAY_HOURS));
	//lock before decrementing
	pthread_mutex_lock(&hunger_mutex);
	vpet->hunger--;
        check_bounds(HUNGER);
	pthread_mutex_unlock(&hunger_mutex);
    }
    return NULL;
}

//Happiness dropping one bar every 8 hours
void *decrease_happiness(void *arg) {
    bool *running = (bool *)arg;
    while (*running) {
        sleep(HOURS(HAPPINESS_DECAY_HOURS));
	pthread_mutex_lock(&happiness_mutex);
	vpet->happiness--;
        check_bounds(HAPPINESS);
	pthread_mutex_unlock(&happiness_mutex);
    }
    return NULL;
}
