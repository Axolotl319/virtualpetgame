#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pet.h"
#include "user-actions.h"

extern pet vpet;

int feed(void) {
	vpet->hunger++;
	check_bounds();
	return EXIT_SUCCESS;
       //I can't think of a failure situation - feel free to correct	
}

int play(void) {
	vpet->happiness++;
	check_bounds();
	return EXIT_SUCCESS; 
}

int clean(void) {
	vpet->cleanliness++;
	check_bounds();
	return EXIT_SUCCESS; 
}

//returns 1 if not enough money, else 0
int gift(void) {
	if(vpet->coins <= 0){
		fprintf(stdout, "%s is out of money!", vpet->name);
		return 1;
	}
	//will work out gift design
	return EXIT_SUCCESS; 
}	
