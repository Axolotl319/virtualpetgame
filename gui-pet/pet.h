#define MAX_HEARTS 5
#define MIN_HEARTS 0
#define MAX_LEVEL 10
#define CLEANLINESS_DECAY_HOURS 12
#define HUNGER_DECAY_HOURS 5
#define HAPPINESS_DECAY_HOURS 8

typedef struct {
	int level_num; 
	int num_actions; 
} level; 

struct pet {
	char *name; 
	unsigned int cleanliness;
	unsigned int happiness;
	unsigned int hunger;
	unsigned int max_hearts;
	unsigned int num_actions;
	level curr_level;
	bool level_up;
	bool alive;
};

typedef struct pet *pet;

#ifndef VPET_GLOBAL
#define VPET_GLOBAL
//global variable - the pet the user will interact with
extern pet vpet;
#endif

typedef enum {
	CLEANLINESS,
	HAPPINESS,
	HUNGER,
} stats;

extern const level levels[MAX_LEVEL - 1]; 

//if any hearts are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(void);

extern void print_hearts(void);

extern void free_pet(void); 

extern pet new_pet(char *name); 

extern void print_warning(int category);

extern void increase_level(void);

extern int check_health(void *app);

extern int dec_cleanliness(void *app);

extern int dec_happiness(void *app);

extern int dec_hunger(void *app);
