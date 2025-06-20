#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pet.h"
#include "user-actions.h"
#include "coins.h"

#define GIFT_COST 5

//global variables included from other files
extern pet vpet;
extern int coins;

int feed(void) {
	if(vpet->hunger >= MAX_HEARTS){
		fprintf(stdout, "%s is full!\n", vpet->name);
		return EXIT_FAILURE; 
	}else{
		fprintf(stdout, "%s savours the delicious meal\n", vpet->name);
		vpet->hunger++; 
		vpet->num_actions++;
	}
	check_bounds();
	return EXIT_SUCCESS;	
}

int play(void) {
	if(vpet->happiness >= MAX_HEARTS){
		fprintf(stdout, "%s is too tired to play\n", vpet->name);
		return EXIT_FAILURE; 
	}else{
		fprintf(stdout, "%s is super excited to spend time with you!\n", 
			vpet->name);
		vpet->happiness++;
		vpet->num_actions++;
	}
	check_bounds();
	return EXIT_SUCCESS; 
}

int clean(void) {
	if(vpet->cleanliness >= MAX_HEARTS){
		fprintf(stdout, "%s is already squeaky clean!\n", vpet->name);
	}else{
		fprintf(stdout, "%s calmly enjoys the bubbles\n", vpet->name);
		vpet->cleanliness++;
		vpet->num_actions++;
	}
	check_bounds();
	return EXIT_SUCCESS; 
}

//returns FAILURE if not enough money or error, else SUCCESS
int gift(void) {
	if(decrement_coins(GIFT_COST)){
		fprintf(stdout, "%s doesn't have enough money!\n", vpet->name);
		return EXIT_FAILURE;
	}
	
	vpet->happiness = MAX_HEARTS;
	fprintf(stdout, "%s is super happy! Thanks for the gift!\n", vpet->name);
	vpet->num_actions++;

	return EXIT_SUCCESS; 
}


