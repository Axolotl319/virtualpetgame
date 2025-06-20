#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
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


static void *take_input( void *arg ) {
	bool *running = (bool *)arg; 
	//set input flags to be non-blocking
	//need this so that the program can terminate (doesn't hang on getchar)
	int flags = fcntl(STDIN_FILENO, F_GETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	while(*running) {
		int c = toupper(getchar());
		
		switch(c) {
			case 'S':
				print_hearts();
				break;
			case 'F':
				feed();
				break;
			case 'C':
				clean();
				break;
			case 'P':
				play();
				break;
			case 'G':
				gift();
				break;
		}	
	}

	//restore the original flags
	fcntl(STDIN_FILENO, F_SETFL, flags);

	return NULL;
}


int main(void) {

	init(); 
	printf("%s is happy to meet you!\n", vpet->name); 
	printf("Press F to feed, P to play, C to clean, G to gift, S for stats\n");

	bool running = 1;

	//initialise money
	coins = 15;
		
 	//thread for incrementing coins
	pthread_t coin_thread;
	pthread_create(&coin_thread, NULL, increment_coins, (void*)&running);

	//thread for checking user input
	pthread_t input_thread;
	pthread_create(&input_thread, NULL, take_input, (void*)&running);
	
	//threads to decrease levels
	pthread_t cleanliness_thread, hunger_thread, happiness_thread;
	pthread_create(&cleanliness_thread, NULL, decrease_cleanliness, (void*)&running);
	pthread_create(&hunger_thread, NULL, decrease_hunger, (void*)&running);
	pthread_create(&happiness_thread, NULL, decrease_happiness, (void*)&running);

	bool clean_warning_reported = false;
	bool happy_warning_reported = false;
	bool hunger_warning_reported = false;

	while(1) {
	
		pthread_mutex_lock(&vpet_mutex);
		//break out of the loop if it's dead
		if (!vpet->alive) {
			printf("Your virtual pet is dead :(\n");
			pthread_mutex_unlock(&vpet_mutex);
			break;
		}

		//warn user if any category has 1 heart remaining
		if(vpet->cleanliness == 1 && !clean_warning_reported){
			print_warning(CLEANLINESS);
			
			//prevents warning being printed repeatedly while
			//vpet->cleanliness == 1
			clean_warning_reported = true;
		}else if(vpet->cleanliness > 1){
			//reset if user increases cleanliness
			clean_warning_reported = false;
		}
		if(vpet->happiness == 1 && !happy_warning_reported){
			print_warning(HAPPINESS);
			happy_warning_reported = true;
		}else if(vpet->happiness > 1){
			happy_warning_reported = false;
		}
		if(vpet->hunger == 1 && !hunger_warning_reported){
			print_warning(HUNGER);
			hunger_warning_reported = true;
		}else if(vpet->hunger > 1){
			hunger_warning_reported = false;
		}

		//check if level needs to be updated
		if (vpet->curr_level.level_num < MAX_LEVEL &&
		    vpet->num_actions >= vpet->curr_level.num_actions) {
			increase_level();
		}	

		//dies if any stat = 0 (or all, can change)
		check_bounds();
		vpet->alive = ((vpet->cleanliness > 0) && 
			       (vpet->happiness > 0)   && 
			       (vpet->hunger > 0));
		
		pthread_mutex_unlock(&vpet_mutex);
	}

	//join the coin thread if infinite loop exited
	running = 0;
	pthread_join(coin_thread, NULL);
	pthread_join(input_thread, NULL);
	pthread_join(cleanliness_thread, NULL);
	pthread_join(hunger_thread, NULL);
	pthread_join(happiness_thread, NULL);
	
	free_pet(); 
        return EXIT_SUCCESS;
}


