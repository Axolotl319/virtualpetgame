#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "pet.h"
#include "coins.h"
#include "user-actions.h"

#define MAX_NAME_LEN 100

static void init(void) {
	printf("Welcome to your virtual pet!\n"); 
	printf("Name your pet (100 characters max): ");
	char petname[MAX_NAME_LEN]; 
	scanf("%s", petname); 
	while (strlen(petname) >= MAX_NAME_LEN) {
		printf("Sorry, that name is too long! Please try again: "); 
		scanf("%s", petname); 
	}
	vpet = new_pet(petname);
}


int main(void) {

	init(); 
	printf("%s is happy to meet you!\n", vpet->name);

	bool running = 1;

	//initialise money
	coins = 0;
		
 	//thread for incrementing coins
	pthread_t coin_thread;
	pthread_create(&coin_thread, NULL, increment_coins, (void*)&running);
	
	while(1) {
		//temporary code here
		printf("Sleeping....\n");
		sleep(2);

		//break out of the loop if it's dead
		if (!vpet->alive) {
			printf("Your virtual pet is dead :(\n");
			break;
		}
	}

	//join the coin thread if infinite loop exited
	running = 0;
	pthread_join(coin_thread, NULL);
	
	free_pet(); 
        return EXIT_SUCCESS;
}
