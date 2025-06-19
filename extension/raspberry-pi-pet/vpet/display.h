#define DEFAULT_CAT_PATH "cat_images/default.bmp"
#define FEED_CAT_PATH    "cat_images/feed.bmp"
#define HAPPY_CAT_PATH   "cat_images/happy.bmp"
#define CLEAN_CAT_PATH   "cat_images/clean.bmp"
#define GIFT_CAT_PATH    "cat_images/gift.bmp"
#define SLEEP_CAT_PATH   "cat_images/sleep.bmp"

extern UWORD *CatImage;

extern void init_display ( void );
extern void display_cat ( void );
extern void display_action_cat ( char *filename);
extern void display_hearts( void );
extern void free_display( void );
