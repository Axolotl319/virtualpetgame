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
// returns 1 if fail, 0 if success
int decrement_coins( int amount ) {
	if (coins >= amount) {
		coins -= amount;
	        printf("You bought a gift for five coins. "); 
		printf("You have %d coins remaining.\n", coins); 	
		return EXIT_SUCCESS; 
	}
	return EXIT_FAILURE;
}


