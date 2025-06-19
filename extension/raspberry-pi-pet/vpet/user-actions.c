#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "pet.h"
#include "user-actions.h"
#include "coins.h"

//global variables included from other files
extern pet vpet;
extern int coins;

void feed(void) {
	pthread_mutex_lock(&hunger_mutex);
	if(vpet->hunger++ >= vpet->max_hearts){
		fprintf(stdout, "%s is full!\n", vpet->name);
	}else{
		fprintf(stdout, "%s savours the delicious meal\n", vpet->name);
		vpet->num_actions++;
	}
	check_bounds(HUNGER);	
	pthread_mutex_unlock(&hunger_mutex);
}

void play(void) {
	pthread_mutex_lock(&happiness_mutex);
	if(vpet->happiness++ >= vpet->max_hearts){
		fprintf(stdout, "%s is too tired to play\n", vpet->name);
	}else{
		fprintf(stdout, "%s is super excited to spend time with you!\n", vpet->name);
		vpet->num_actions++;
	}
	check_bounds(HAPPINESS); 
	pthread_mutex_unlock(&happiness_mutex);
}

void clean(void) {
	pthread_mutex_lock(&cleanliness_mutex);
	if(vpet->cleanliness++ >= vpet->max_hearts){
		fprintf(stdout, "%s is already squeaky clean!\n", vpet->name);
	}else{
		fprintf(stdout, "%s calmly enjoys the bubbles\n", vpet->name);
		vpet->num_actions++;
	}
	check_bounds(CLEANLINESS); 
	pthread_mutex_unlock(&cleanliness_mutex);
}

void gift(void) {

	if(decrement_coins(5)){
		fprintf(stdout, "%s doesn't have enough money!\n", vpet->name);
		return;
	}
	
	pthread_mutex_lock(&happiness_mutex);
	vpet->happiness = vpet->max_hearts;
	pthread_mutex_unlock(&happiness_mutex);

	fprintf(stdout, "%s is super happy! Thanks for the gift!\n", vpet->name);
	vpet->num_actions++; 
}
