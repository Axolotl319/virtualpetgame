#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "coins.h"

//variable for money
int coins;
//lock for incrementing/decrementing coins
pthread_mutex_t coins_mutex = PTHREAD_MUTEX_INITIALIZER;

// increments the coin variable every set interval
void *increment_coins( void *arg ) {
	bool *running = (bool *)arg;
	while(*running) {
		sleep(COIN_TIME);
		//lock before incrementing coins
		pthread_mutex_lock(&coins_mutex);
		coins += COIN_AMT;
		//unlock
		pthread_mutex_unlock(&coins_mutex);

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
	}
	//unlock
	pthread_mutex_unlock(&coins_mutex);
	return success;
}


