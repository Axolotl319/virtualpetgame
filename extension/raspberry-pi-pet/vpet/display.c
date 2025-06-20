#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "pet.h"
#include "coins.h"
#include "display.h"

//constants for displaying text
#define NEWLINE_GAP_24 25
#define NEWLINE_GAP_48 60
#define LEFT_INDEX     5
#define TOP_24         20
#define TOP_48         40
#define CHAR_SPACE     24
#define WIDTH_48       24
#define HEIGHT_48      48

UWORD *CatImage;

//initialise the display
void init_display( void ) {
        // Exception handling:ctrl + c
        signal(SIGINT, Handler_1IN3_LCD);
    
        // Module Init 
                if(DEV_ModuleInit() != 0){
                DEV_ModuleExit();
                exit(0);
        }

	//initialise and clear screen
        LCD_1IN3_Init(HORIZONTAL);
        LCD_1IN3_Clear(WHITE);
        LCD_SetBacklight(1023);

	//allocate memory
        UDOUBLE Imagesize = LCD_1IN3_HEIGHT*LCD_1IN3_WIDTH*2;
        printf("Imagesize = %d\r\n", Imagesize);
        if((CatImage = (UWORD *)malloc(Imagesize)) == NULL) {
                printf("Failed to apply for memory...\r\n");
                exit(0);
        }

	//create a new image
        Paint_NewImage(CatImage, LCD_1IN3_WIDTH, LCD_1IN3_HEIGHT, 0, WHITE, 16);
	Paint_SetRotate(ROTATE_90);

        display_start();
}

//displays the start screen 
void display_start(void) {
	Paint_Clear(WHITE);
	LCD_1IN3_Clear(WHITE);

	//welcome message
	Paint_DrawString_EN(4*LEFT_INDEX, TOP_48, "WELCOME", &Font48, WHITE, BLACK);
	Paint_DrawString_EN(4*LEFT_INDEX, TOP_48 + NEWLINE_GAP_48, "TO YOUR", &Font48, WHITE, BLACK);
	Paint_DrawString_EN(10*LEFT_INDEX, TOP_48 + 2*NEWLINE_GAP_48, "PET!", &Font48, WHITE, BLACK);
	LCD_1IN3_Display(CatImage);
	DEV_Delay_ms(2000);
	
	//screen to enter name
	display_name_pg();
}

//screen to enter name
void display_name_pg(void) {
	Paint_Clear(WHITE);
        Paint_DrawString_EN(10*LEFT_INDEX, TOP_48, "NAME:", &Font48, WHITE, BLACK);
        LCD_1IN3_Display(CatImage);
}

//displays a character once it is typed as a name
void display_name_char(char c, int pos) {
	Paint_DrawChar(CHAR_SPACE*pos, TOP_48 + NEWLINE_GAP_48, c, &Font48, BLACK, WHITE);
	LCD_1IN3_Display(CatImage);
}

//removes a character when backspace is pressed
void remove_name_char(int pos) {
	Paint_DrawRectangle(CHAR_SPACE * pos, 
		    	    TOP_48 + NEWLINE_GAP_48,
			    CHAR_SPACE * pos + WIDTH_48, 
			    TOP_48 + NEWLINE_GAP_48 + HEIGHT_48,	
			    WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	LCD_1IN3_Display(CatImage);
}

//displays the default cat image
void display_cat(void) {
	Paint_Clear(WHITE);
	LCD_1IN3_Clear(WHITE);
	GUI_ReadBmp(DEFAULT_CAT_PATH);
        LCD_1IN3_Display(CatImage);
}

//displays an image for 2 seconds
void display_temp_img(char *filename) {
	Paint_Clear(WHITE);
	LCD_1IN3_Clear(WHITE);
	GUI_ReadBmp(filename);
	LCD_1IN3_Display(CatImage);
	DEV_Delay_ms(2000);
	display_cat();
}

//displays the statistics of the pet
void display_hearts(void) {
	Paint_Clear(WHITE);

	//pass all the information into a string
	char level_str[10];
	sprintf(level_str, "Level: %d", vpet->curr_level.level_num);	

	char coins_str[50];
	sprintf(coins_str, "Coins: %d", coins);

	char cleanliness_str[MAX_STAT_LEN];
	char happiness_str[MAX_STAT_LEN];
	char hunger_str[MAX_STAT_LEN];

	get_stat_string(CLEANLINESS, vpet->cleanliness, cleanliness_str);
	get_stat_string(HAPPINESS, vpet->happiness, happiness_str);
	get_stat_string(HUNGER, vpet->hunger, hunger_str);

        Paint_DrawString_EN(LEFT_INDEX, TOP_24, level_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + NEWLINE_GAP_24, "Cleanliness:", &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + 2*NEWLINE_GAP_24, cleanliness_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + 3*NEWLINE_GAP_24, "Happiness:", &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + 4*NEWLINE_GAP_24, happiness_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + 5*NEWLINE_GAP_24, "Hunger:", &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + 6*NEWLINE_GAP_24, hunger_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(LEFT_INDEX, TOP_24 + 7*NEWLINE_GAP_24, coins_str, &Font24, WHITE, BLACK);

        LCD_1IN3_Display(CatImage);
        DEV_Delay_ms(2000);
        display_cat();
}

//displays the death message
void display_death_msg(void) {
	Paint_Clear(WHITE);
	Paint_DrawString_EN(LEFT_INDEX, TOP_48, "YOUR PET", &Font48, WHITE, BLUE);
	Paint_DrawString_EN(3*LEFT_INDEX, TOP_48 + NEWLINE_GAP_48, "IS DEAD", &Font48, WHITE, BLUE);
	Paint_DrawString_EN(18*LEFT_INDEX, TOP_48 + 2*NEWLINE_GAP_48, ":(", &Font48, WHITE, BLUE);
	LCD_1IN3_Display(CatImage);
}

//frees memory for the display
void free_display(void){
	free(CatImage);
	CatImage = NULL;
}
