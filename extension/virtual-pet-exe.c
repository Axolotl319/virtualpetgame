#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pet.h"
#include "user-actions.h"

pet vpet; 

static void init(void) {
	printf("Welcome to your virtual pet!\n"); 
	printf("Name your pet (100 characters max): ");
	char petname[100]; 
	scanf("%s", petname); 
	while (strlen(petname) >= 100) {
		printf("Sorry, that name is too long! Please try again: "); 
		scanf("%s", petname); 
	}
	vpet = new_pet(petname);
}

int main(void) {

	init(); 
	printf("%s is happy to meet you!\n", vpet->name); 
 	
	/*
         * INFINITE LOOP TO BE IMPLEMENTED
         */

	free_pet(); 
        return EXIT_SUCCESS;
}
