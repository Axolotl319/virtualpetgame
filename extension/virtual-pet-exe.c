#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pet.h"
#include "user-actions.h"

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
	while(vpet->alive){

		//warn user if any category has 1 heart remaining
		if(vpet->cleanliness == 1){
			print_warning(0);
		}
		if(vpet->happiness == 1){
			print_warning(1);
		}
		if(vpet->hunger == 1){
			print_warning(2);
		}

	}

	free_pet(); 
        return EXIT_SUCCESS;
}
