#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "DEV_Config.h"
#include "LCD_1in3.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "pet.h"
#include "coins.h"

//Helper macro to convert hours to seconds
#define HOURS(x) ((x) * 3600)


//external global variable
extern int coins;

//global variable - the pet the user will interact with
pet vpet;

//const array for level information
const level levels[MAX_LEVEL - 1] = {
	{ 1,  5},
	{ 2,  15},
	{ 3,  30},
	{ 4,  50},
	{ 5,  75},
	{ 6,  105},
	{ 7,  140},
	{ 8,  180},
	{ 9,  225},
};


void check_bounds(void){
	if(vpet->cleanliness > vpet->max_hearts){
		vpet->cleanliness = vpet->max_hearts;
		printf("debug: cleanliness set to > 5, reset to 5\n");
	}else if(vpet->cleanliness < 0){
		vpet->cleanliness = 0;
		printf("debug: cleanliness set to < 0, reset to 0\n");
	}

	if(vpet->happiness > vpet->max_hearts){
		vpet->happiness = vpet->max_hearts;
		printf("debug: happiness set to > 5, reset to 5\n");
	}else if(vpet->happiness < 0){
		vpet->happiness = 0;
		printf("debug: happiness set to < 0, reset to 0\n");
	}

	if(vpet->hunger > vpet->max_hearts){
		vpet->hunger = vpet->max_hearts;
		printf("debug: hunger set to > 5, reset to 5\n");
	}else if(vpet->hunger < 0){
		vpet->hunger = 0;
		printf("debug: hunger set to < 0, reset to 0\n");
	}
}

static void print_stat(int category, int amt){
	switch(category){
		case CLEANLINESS: 
			fprintf(stdout, "Cleaniness ");
			break;

		case HAPPINESS: 
			fprintf(stdout, "Happiness ");
			break;

		case HUNGER: 
			fprintf(stdout, "Hunger ");
			break;
	
		default: 
			fprintf(stderr, "Unknown category\n");
			return;
			break;
	}
	for(int i = 0; i < amt; i++){
		fprintf(stdout, "+");
	}
	fprintf(stdout, "\n");
}

void print_hearts(void) {
	fprintf(stdout, "Stats:\n");
	check_bounds(); //ensure all stats between 0-5
	fprintf(stdout, "Level %d\n", vpet->curr_level.level_num);
	print_stat(CLEANLINESS, vpet->cleanliness);
	print_stat(HAPPINESS, vpet->happiness);
	print_stat(HUNGER, vpet->hunger);
	fprintf(stdout, "%d coins\n", coins);
}

void print_hearts_pi(void) {
	signal(SIGINT, Handler_1IN3_LCD);
    
   	 /* Module Init */
        if(DEV_ModuleInit() != 0){
        	DEV_ModuleExit();
        	exit(0);
    	}
	UWORD *BlackImage;
    	UDOUBLE Imagesize = LCD_1IN3_HEIGHT*LCD_1IN3_WIDTH*2;
    	printf("Imagesize = %d\r\n", Imagesize);
    	if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        	printf("Failed to apply for black memory...\r\n");
        	exit(0);
    	}
    	// /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    	Paint_NewImage(BlackImage, LCD_1IN3_WIDTH, LCD_1IN3_HEIGHT, 0, WHITE, 16);
    	Paint_Clear(WHITE);
        Paint_SetRotate(ROTATE_90);

	Paint_DrawString_EN(5, 30, "Stats:", &Font24, WHITE, BLACK);
    	Paint_DrawString_EN(5, 60, "Level:", &Font24, WHITE, BLACK);
	LCD_1IN3_Display(BlackImage);
    	DEV_Delay_ms(2000);
	free(BlackImage);
    	BlackImage = NULL;
}

void print_warning(int category){
	fprintf(stdout, "Warning! Only 1 heart remaining for ");
	switch(category){
		case CLEANLINESS: 
			fprintf(stdout, "cleanliness. Press C to boost!");
			return;

		case HAPPINESS: 
			fprintf(stdout, "happiness. Press P to boost!");
			return;

		case HUNGER: 
			fprintf(stdout, "hunger. Press F to boost!");
			return;

		default: fprintf(stderr, "UNKNOWN CATEGORY");
			 return;
	}
}

void increase_level( void ) {
	if (vpet->curr_level.level_num < MAX_LEVEL) {
		vpet->curr_level = levels[vpet->curr_level.level_num++];
		vpet->max_hearts++;
		vpet->num_actions = 0;
		vpet->level_up = false;
		fprintf(stdout, "%s has levelled up!\n", vpet->name);
	}
}

pet new_pet(char *name) {
	pet new = malloc(sizeof(struct pet));
	new->name = strdup(name);
	new->cleanliness = INIT_HEARTS;
	new->happiness   = INIT_HEARTS;
	new->hunger      = INIT_HEARTS;
	new->max_hearts  = INIT_HEARTS;
	new->curr_level  = levels[0];	
	new->num_actions = 0;
	new->level_up    = false;
	new->alive       = true;
	return new;
}

void free_pet(void) {
	free(vpet);
}

//Cleanliness droppine one bar every 12 hours
void *decrease_cleanliness(void *arg) {
    bool *running = (bool *)arg;
    while (*running && vpet->alive) {
        sleep(HOURS(CLEANLINESS_DECAY_HOURS));
        vpet->cleanliness--;
        check_bounds();
    }
    return NULL;
}

//Hunger dropping one bar every 5 hours
void *decrease_hunger(void *arg) {
    bool *running = (bool *)arg;
    while (*running && vpet->alive) {
        sleep(HOURS(HUNGER_DECAY_HOURS));
        vpet->hunger--;
        check_bounds();
    }
    return NULL;
}

//Happiness dropping one bar every 8 hours
void *decrease_happiness(void *arg) {
    bool *running = (bool *)arg;
    while (*running && vpet->alive) {
        sleep(HOURS(HAPPINESS_DECAY_HOURS));
        vpet->happiness--;
        check_bounds();
    }
    return NULL;
}
