#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include "pet.h"
#include "coins.h"
#include "user-actions.h"

#define MAX_NAME_LEN 100

static void activate(GtkApplication *app, gpointer user_data) {
	// Setup the GUI window 
	GtkWidget *window; 
	window = gtk_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(window), "Your Virtual Pet"); 
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// Add the default pet image to GUI window 
	GtkWidget *image; 
	image = gtk_image_new_from_file("cat.jpg"); 
	gtk_container_add(GTK_CONTAINER(window), image); 	

	// Add the default pet to GUI window
	gtk_widget_show_all(window); 
}

static int init(void) {
	// Setup pet personalization 
	printf("Welcome to your virtual pet!\n"); 
	printf("Name your pet (100 characters max): ");
	char petname[MAX_NAME_LEN]; 
	scanf("%s", petname); 
	while (strlen(petname) >= MAX_NAME_LEN) {
		printf("Sorry, that name is too long! Please try again: "); 
		scanf("%s", petname); 
	}
	vpet = new_pet(petname);

	// Initialise and run the GUI application in a different thread
	GtkApplication *app; 
	int ret; 
	app = gtk_application_new("in.virtualpet", G_APPLICATION_DEFAULT_FLAGS); 
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL); 
	ret = g_application_run(G_APPLICATION(app), 0, NULL); 
	g_object_unref(app); 
	return ret; 
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
				printf("Hunger\n");
				feed();
				break;
			case 'C':
				printf("Clean\n");
				clean();
				break;
			case 'P':
				printf("Play\n");
				play();
				break;
			case 'G':
				printf("Gift\n");
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

	bool running = 1;

	//initialise money
	coins = 0;
		
 	//thread for incrementing coins
	pthread_t coin_thread;
	pthread_create(&coin_thread, NULL, increment_coins, (void*)&running);

	//thread for checking user input
	pthread_t input_thread;
	pthread_create(&input_thread, NULL, take_input, (void*)&running);
	
	while(1) {
		//temporary code here
		//printf("Sleeping....\n");
		//sleep(15);
		//vpet->alive = false;

		//break out of the loop if it's dead
		if (!vpet->alive) {
			printf("Your virtual pet is dead :(\n");
			break;
		}

		//warn user if any category has 1 heart remaining
		if(vpet->cleanliness == 1){
			print_warning(CLEANLINESS);
		}
		if(vpet->happiness == 1){
			print_warning(HAPPINESS);
		}
		if(vpet->hunger == 1){
			print_warning(HUNGER);
		}

		//dies if any stat = 0 (or all, can change)
		check_bounds();
		vpet->alive = ((vpet->cleanliness > 0) && (vpet->happiness > 0) && (vpet->hunger > 0));
	}

	//join the coin thread if infinite loop exited
	running = 0;
	pthread_join(coin_thread, NULL);
	pthread_join(input_thread, NULL);
	
	
	free_pet(); 
        return EXIT_SUCCESS;
}
