#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pet.h"
#include "user-actions.h"
#include "coins.h"

//global variables included from other files
extern pet vpet;
extern int coins;

int feed(void) {
	if(vpet->hunger++ >= vpet->max_hearts){
		fprintf(stdout, "%s is full!\n", vpet->name);
	}else{
		fprintf(stdout, "%s savours the delicious meal\n", vpet->name);
		vpet->num_actions++;
	}
	check_bounds();
	return EXIT_SUCCESS;
       //I can't think of a failure situation - feel free to correct	
}

int play(void) {
	if(vpet->happiness++ >= vpet->max_hearts){
		fprintf(stdout, "%s is too tired to play\n", vpet->name);
	}else{
		fprintf(stdout, "%s is super excited to spend time with you!\n", vpet->name);
		vpet->num_actions++;
	}
	check_bounds();
	return EXIT_SUCCESS; 
}

int clean(void) {
	if(vpet->cleanliness++ >= vpet->max_hearts){
		fprintf(stdout, "%s is already squeaky clean!\n", vpet->name);
	}else{
		fprintf(stdout, "%s calmly enjoys the bubbles\n", vpet->name);
		vpet->num_actions++;
	}
	check_bounds();
	return EXIT_SUCCESS; 
}

//returns 1 if not enough money or error, else 0
int gift(void) {

	if(decrement_coins(5)){
		fprintf(stdout, "%s doesn't have enough money!\n", vpet->name);
		return EXIT_FAILURE;
	}
	
	vpet->happiness = vpet->max_hearts;
	fprintf(stdout, "%s is super happy! Thanks for the gift!\n", vpet->name);
	vpet->num_actions++;
	//will work out gift design
	return EXIT_SUCCESS; 
}
