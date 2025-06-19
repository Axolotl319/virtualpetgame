#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "pet.h"
#include "coins.h"
#include "user-actions.h"
#include "display.h"

#define MAX_NAME_LEN 100
#define WAIT_TIME_MICROS 200000

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
	init_display();
}


static void *take_input( void *arg ) {
	bool *running = (bool *)arg; 

	while(*running) {
		//keypress actions
		//need to sleep for some milliseconds to avoid 
		//repeatedly calling the functions

		if(GET_KEY_UP == 0) {
			while(*running && GET_KEY_UP == 0) {
				display_hearts();
			}
		}

		if(GET_KEY_PRESS == 0) {
			while(*running && GET_KEY_PRESS == 0) {
				usleep(WAIT_TIME_MICROS);
				gift();
			}
		}

		if(GET_KEY1 == 0){
			while(*running && GET_KEY1 == 0) {
				usleep(WAIT_TIME_MICROS);
				clean();
			}
		}

		if(GET_KEY2 == 0){
			while(*running && GET_KEY2 == 0) {
				usleep(WAIT_TIME_MICROS);
				play();
			}
		}

		if(GET_KEY3 == 0){
			while(*running && GET_KEY3 == 0) {
				usleep(WAIT_TIME_MICROS);
				feed();
			}
		}
	}


	return NULL;
}


int main(void) {

	init(); 
	printf("%s is happy to meet you!\n", vpet->name); 

	bool running = 1;

	//initialise money
	coins = 0;

	//bools for printing warnings
	bool cleanliness_warning = false;
	bool happiness_warning = false;
	bool hunger_warning = false;
		
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

	while(1) {

		//break out of the loop if it's dead
		if (!vpet->alive) {
			printf("Your virtual pet is dead :(\n");
			break;
		}

		//warn user if any category has 1 heart remaining
		if(vpet->cleanliness == 1){
			if(!cleanliness_warning) {
				cleanliness_warning = true;
				print_warning(CLEANLINESS);
				display_temp_img(CLEANLINESS_WARNING_PATH);
			}
		} else {
			cleanliness_warning = false;
		}

		if(vpet->happiness == 1){
			if(!happiness_warning) {
				happiness_warning = true;
				print_warning(HAPPINESS);
				display_temp_img(HAPPINESS_WARNING_PATH);
			}
		} else {
			happiness_warning = false;
		}

		if(vpet->hunger == 1){
			if(!hunger_warning) {
				hunger_warning = true;
				print_warning(HUNGER);
				display_temp_img(HUNGER_WARNING_PATH);
			}
		} else {
			hunger_warning = false;
		}

		//check if level needs to be updated
		if (vpet->curr_level.level_num < MAX_LEVEL &&
		    vpet->num_actions >= vpet->curr_level.num_actions) {
			increase_level();
		}

		//dies if any stat = 0 (or all, can change)
		vpet->alive = ((vpet->cleanliness > 0) && 
			       (vpet->happiness > 0)   && 
			       (vpet->hunger > 0));

		usleep(50000);
	}

	//join the coin thread if infinite loop exited
	running = 0;
	pthread_join(coin_thread, NULL);
	pthread_join(input_thread, NULL);
	pthread_join(cleanliness_thread, NULL);
	pthread_join(hunger_thread, NULL);
	pthread_join(happiness_thread, NULL);

	free_display();
	free_pet(); 
        return EXIT_SUCCESS;
}
