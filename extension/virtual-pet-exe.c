#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "pet.h"
#include "user-actions.h"

#define MAX_NAME_LEN 100
#define COIN_TIME 4
#define COIN_AMT  5

//variable for money
int coins;
//lock for incrementing/decrementing coins
pthread_mutex_t coins_mutex = PTHREAD_MUTEX_INITIALIZER;
//status of main loop
static int running = 1;

static void init(void) {
	printf("Welcome to your virtual pet!\n"); 
	printf("Name your pet (100 characters max): ");
	char petname[MAX_NAME_LEN]; 
	scanf("%s", petname); 
	while (strlen(petname) >= MAX_NAME_LEN) {
		printf("Sorry, that name is too long! Please try again: "); 
		scanf("%s", petname); 
	}
	vpet = new_pet(petname);
}

// increments the coin variable every set interval
static void *increment_coins( void *arg ) {
	while(running) {
		sleep(COIN_TIME);
		//lock before incrementing coins
		pthread_mutex_lock(&coins_mutex);
		coins += COIN_AMT;
		//unlock
		pthread_mutex_unlock(&coins_mutex);

		printf("DEBUG: Coins: %d\n", coins);
		fflush(stdout);
	}
	return NULL;
}

// decrements the coin variable by a set amount
// checks whether there is enough money
// returns 1 if fail, 0 if success
int decrement_coins( int amount ) {
	int success = EXIT_FAILURE;

	//lock before decrementing
	pthread_mutex_lock(&coins_mutex);
	if (coins >= amount) {
		coins -= amount;
		success = EXIT_SUCCESS;
		printf("DEBUG: Successfully decremented coins\n");
	}
	//unlock
	pthread_mutex_unlock(&coins_mutex);
	return success;
}

int main(void) {

	init(); 
	printf("%s is happy to meet you!\n", vpet->name);

	//initialise money
	coins = 0;
		
 	//thread for incrementing coins
	pthread_t coin_thread;
	pthread_create(&coin_thread, NULL, increment_coins, NULL);

	while(1) {
		//temporary code here
		printf("Sleeping....\n");
		sleep(2);

		//break out of the loop if it's dead
		if (!vpet->alive) {
			printf("Your virtual pet is dead :(\n");
			break;
		}
	}

	//join the coin thread if infinite loop exited
	running = 0;
	pthread_join(coin_thread, NULL);
	
	free_pet(); 
        return EXIT_SUCCESS;
}
