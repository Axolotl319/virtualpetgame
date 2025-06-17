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
	if(vpet->hunger++ >= 5){
		fprintf(stdout, "%s is full!", vpet->name);
	}else{
		fprintf(stdout, "%s savours the delicious meal", vpet->name);
	}
	check_bounds();
	return EXIT_SUCCESS;
       //I can't think of a failure situation - feel free to correct	
}

int play(void) {
	if(vpet->happiness++ >= 5){
		fprintf(stdout, "%s is too tired to play", vpet->name);
	}else{
		fprintf(stdout, "%s is super excited to spend time with you!", vpet->name);
	}
	check_bounds();
	return EXIT_SUCCESS; 
}

int clean(void) {
	if(vpet->cleanliness++ >= 5){
		fprintf(stdout, "%s is already squeaky clean!", vpet->name);
	}else{
		fprintf(stdout, "%s calmly enjoys the bubbles", vpet->name);
	}
	check_bounds();
	return EXIT_SUCCESS; 
}

//returns 1 if not enough money or error, else 0
int gift(void) {
	if(coins <= 0){
		fprintf(stdout, "%s doesn't have enough money!", vpet->name);
		return 1;
	}
	if(coins >= 5){
		vpet->happiness = 5;
		fprintf(stdout, "%s is super happy! Thanks for the gift!", vpet->name);
		if(decrement_coins(5)){
			fprintf(stderr, "Can't decrement coins in gift function");
			return EXIT_FAILURE;
		}
	}
	//will work out gift design
	return EXIT_SUCCESS; 
}
