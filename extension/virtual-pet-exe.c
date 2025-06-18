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

// In case of a key press event, carries out the action associated with the key. 
bool action_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	switch (event->keyval) {
		case (GDK_KEY_s): 
			printf("S key pressed!\n"); 
			print_hearts(); 
			break; 
		case (GDK_KEY_f): 
			printf("F key pressed!\n");
		        feed(); 	
			break;  
		case (GDK_KEY_c): 
			printf("C key pressed!\n"); 
			clean(); 
			break;  
		case (GDK_KEY_p): 
			printf("P key pressed!\n"); 
			play(); 
			break;  
		case (GDK_KEY_g): 
			printf("G key pressed!\n"); 
			gift(); 
			break;  
		default: 
			printf("Not recognized\n");
		       	/* NO OP */ 
			break;  	
	}
	return true; 
}

// Function that checks the pet's health and issues warnings 
// This is called by GTK's main function, as set up in activate()
// Returns FALSE if pet dies, TRUE otherwise 
// When FALSE is returned, the function is never called again 
static int check_health(void *app) {
	//dies if any stat = 0 (or all, can change)
	check_bounds();
	vpet->alive = ((vpet->cleanliness > 0) && (vpet->happiness > 0) && (vpet->hunger > 0));	
	
	//quit application if pet dies
	if (!vpet->alive) {
		printf("Your virtual pet is dead :(\n");
		g_application_quit(G_APPLICATION(app)); 
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

	return TRUE; 	
}

// If hunger ever reaches zero, the function is never called again. 
static int dec_hunger(void *app) {
	vpet->hunger--;
        if (vpet->hunger <= 0) {
		return FALSE; 
	}
	return TRUE; 
}

// If cleanliness ever reaches zero, the function is never called again 
static int dec_cleanliness(void *app) {
	vpet->cleanliness--;
       	if (vpet->cleanliness <= 0) {
		return FALSE; 
	}	
	return TRUE; 
}

// If happiness ever reaches zero, the function is never called again 
static int dec_happiness(void *app) {
	vpet->happiness--;
        if (vpet->happiness <= 0) {
		return FALSE; 
	}	
	return TRUE; 
}

// Sets up and starts the GUI application window 
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

	// connect a keyboard press event to the window 
	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK); 
	g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(action_keypress), NULL); 

	// add functions to decrement health and check for death
	g_timeout_add_seconds(5, dec_hunger, NULL); 
	g_timeout_add_seconds(12, dec_cleanliness, NULL); 
	g_timeout_add_seconds(8, dec_happiness, NULL); 
	g_timeout_add_seconds(2, check_health, app); 

	// Show the window and default pet 
	gtk_widget_show_all(window); 
}

static int init(void) {
	// Setup pet personalization 
	printf("Welcome to your virtual pet!\n"); 
	printf("Name your pet (%d characters max): ", MAX_NAME_LEN);
	char petname[MAX_NAME_LEN]; 
	scanf("%s", petname); 
	while (strlen(petname) >= MAX_NAME_LEN) {
		printf("Sorry, that name is too long! Please try again: "); 
		scanf("%s", petname); 
	}
	vpet = new_pet(petname);
	printf("%s is happy to meet you!\n", vpet->name); 

	bool running = 1;

	//initialise money
	coins = 0;
		
 	//thread for incrementing coins
	pthread_t coin_thread;
	pthread_create(&coin_thread, NULL, increment_coins, (void*)&running);

	// Initialise and run the GUI application 
	GtkApplication *app; 
	int ret; 
	app = gtk_application_new("in.virtualpet", G_APPLICATION_DEFAULT_FLAGS); 
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL); 
	ret = g_application_run(G_APPLICATION(app), 0, NULL); 
	g_object_unref(app); 
	
	//join the coin thread if application exited
	running = 0;
	pthread_join(coin_thread, NULL);

	return ret; 
}

int main(void) {
	// initialize and start application 
	init(); 

	// cleanup 
	free_pet(); 
        return EXIT_SUCCESS;
}
