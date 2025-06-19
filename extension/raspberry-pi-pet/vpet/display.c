#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "pet.h"
#include "coins.h"
#include "display.h"

UWORD *CatImage;

void init_display( void ) {
        // Exception handling:ctrl + c
        signal(SIGINT, Handler_1IN3_LCD);
    
        /* Module Init */
                if(DEV_ModuleInit() != 0){
                DEV_ModuleExit();
                exit(0);
        }

        LCD_1IN3_Init(HORIZONTAL);
        LCD_1IN3_Clear(WHITE);
        LCD_SetBacklight(1023);

        UDOUBLE Imagesize = LCD_1IN3_HEIGHT*LCD_1IN3_WIDTH*2;
        printf("Imagesize = %d\r\n", Imagesize);
        if((CatImage = (UWORD *)malloc(Imagesize)) == NULL) {
                printf("Failed to apply for memory...\r\n");
                exit(0);
        }
        Paint_NewImage(CatImage, LCD_1IN3_WIDTH, LCD_1IN3_HEIGHT, 0, WHITE, 16);
        Paint_SetRotate(ROTATE_90);

        display_cat();
}

void display_cat(void) {
	Paint_Clear(WHITE);
	GUI_ReadBmp(DEFAULT_CAT_PATH);
        LCD_1IN3_Display(CatImage);
}

void display_action_cat(char *filename) {
	Paint_Clear(WHITE);
	GUI_ReadBmp(filename);
	LCD_1IN3_Display(CatImage);
	DEV_Delay_ms(2000);
	display_cat();
}

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

        Paint_DrawString_EN(5, 20, level_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 45, "Cleanliness:", &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 70, cleanliness_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 95, "Happiness:", &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 120, happiness_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 145, "Hunger:", &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 170, hunger_str, &Font24, WHITE, BLACK);
	Paint_DrawString_EN(5, 195, coins_str, &Font24, WHITE, BLACK);

        LCD_1IN3_Display(CatImage);
        DEV_Delay_ms(2000);
        display_cat();
}

void free_display(void){
	free(CatImage);
	CatImage = NULL;
}
