#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "coins.h"

//variable for money
int coins;

// increments the coin variable every set interval
int increment_coins( void *arg ) {
	coins += COIN_AMT; 
	return true;
}

// decrements the coin variable by a set amount
// checks whether there is enough money
// returns EXIT_FAILURE if fail, EXIT_SUCCESS if success
int decrement_coins( int amount ) {
	if (coins >= amount) {
		coins -= amount; 
		return EXIT_SUCCESS; 
	}
	return EXIT_FAILURE;
}


