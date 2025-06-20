#define DEFAULT_CAT_PATH 	 "cat_images/default.bmp"
#define FEED_CAT_PATH    	 "cat_images/feed.bmp"
#define HAPPY_CAT_PATH   	 "cat_images/happy.bmp"
#define CLEAN_CAT_PATH   	 "cat_images/clean.bmp"
#define GIFT_CAT_PATH    	 "cat_images/gift.bmp"
#define SLEEP_CAT_PATH   	 "cat_images/sleep.bmp"
#define HUNGER_WARNING_PATH      "cat_images/hunger_warning.bmp"
#define HAPPINESS_WARNING_PATH   "cat_images/happiness_warning.bmp"
#define CLEANLINESS_WARNING_PATH "cat_images/cleanliness_warning.bmp"


extern UWORD *CatImage;

extern void init_display ( void );
extern void display_cat ( void );
extern void display_start ( void );
extern void display_name_pg( void );
extern void display_name_char( char c, int pos );
extern void remove_name_char( int pos );
extern void display_temp_img ( char *filename);
extern void display_hearts( void );
extern void display_warning( int category );
extern void display_death_msg( void );
extern void free_display( void );
