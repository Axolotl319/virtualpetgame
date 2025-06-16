#include <stdio.h>
#include <stdlib.h>
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
	return EXIT_FAILURE; 
}

int gift(void) {
	return EXIT_FAILURE; 
}	
