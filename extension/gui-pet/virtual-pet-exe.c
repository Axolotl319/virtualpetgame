#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "pet.h"
#include "coins.h"
#include "user-actions.h"

#define MAX_NAME_LEN 100
#define CHECK_STAT_INTERVAL 2

// GTK window dimensions
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 400

// In case of a key press event, carries out the action associated with the key. 
// Updates the image of cat for each key press as well 
bool action_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	switch (event->keyval) {
		case (GDK_KEY_s):  
			print_hearts(); 
			gtk_image_set_from_file(GTK_IMAGE(data), "cat_images/default.jpg"); 
			break; 
		case (GDK_KEY_f): 
		        feed();
			gtk_image_set_from_file(GTK_IMAGE(data), "cat_images/feed.png"); 
			break;  
		case (GDK_KEY_c):  
			clean();
		       	gtk_image_set_from_file(GTK_IMAGE(data), "cat_images/clean.png"); 	
			break;  
		case (GDK_KEY_p):  
			if (play()) {
				gtk_image_set_from_file(GTK_IMAGE(data), "cat_images/sleep.png"); 
			} else {
				gtk_image_set_from_file(GTK_IMAGE(data), "cat_images/happy.png"); 
			}	
			break;  
		case (GDK_KEY_g): 
			if (!gift()) {
				gtk_image_set_from_file(GTK_IMAGE(data), "cat_images/gift.png"); 
			}	
			break;  
		default: 
		       	/* NO OP */ 
			break;  	
	}
	printf("\n"); 
	return true; 
}

static int check_level(void *data) {
	if (vpet->curr_level.level_num < MAX_LEVEL &&
	    vpet->num_actions >= vpet->curr_level.num_actions) {
		increase_level(); 
	}
	return TRUE; 
}

// Function that checks the pet's health and issues warnings  
// This is called by GTK's main function, as set up in activate() 
// Returns FALSE if pet dies, TRUE otherwise  
// When FALSE is returned, the function is never called again  
int check_health(void *app) { 
        //dies if any stat falls beneath the minimum  
        check_bounds(); 
        vpet->alive = ((vpet->cleanliness > MIN_HEARTS) && 
			(vpet->happiness > MIN_HEARTS) && 
			(vpet->hunger > MIN_HEARTS)); 
 
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

// Sets up and starts the GUI application window 
static void activate(GtkApplication *app, gpointer user_data) {
	// Setup the GUI window 
	GtkWidget *window; 
	window = gtk_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(window), "Your Virtual Pet"); 
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// Add the default pet image to GUI window 
	GtkWidget *image; 
	image = gtk_image_new_from_file("cat_images/default.jpg"); 
	gtk_container_add(GTK_CONTAINER(window), image); 	

	// connect a keyboard press event to the window 
	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK); 
	g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(action_keypress), image); 

	// add functions to decrement health, check for death, increment coins, and level up 
	g_timeout_add_seconds(HUNGER_DECAY_HOURS, dec_hunger, NULL); 
	g_timeout_add_seconds(CLEANLINESS_DECAY_HOURS, dec_cleanliness, NULL); 
	g_timeout_add_seconds(HAPPINESS_DECAY_HOURS, dec_happiness, NULL); 
	g_timeout_add_seconds(CHECK_STAT_INTERVAL, check_health, app); 
	g_timeout_add_seconds(COIN_TIME, increment_coins, NULL); 
	g_timeout_add_seconds(CHECK_STAT_INTERVAL, check_level, NULL); 

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
        printf("\n"); 	

	//initialise money
	coins = 0;
		
	// Initialise and run the GUI application 
	GtkApplication *app; 
	int ret; 
	app = gtk_application_new("in.virtualpet", G_APPLICATION_DEFAULT_FLAGS); 
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL); 
	ret = g_application_run(G_APPLICATION(app), 0, NULL); 
	g_object_unref(app); 
	
	return ret; 
}

int main(void) {
	// initialize and start application 
	init(); 

	// cleanup 
	free_pet(); 
        return EXIT_SUCCESS;
}
