#define INIT_HEARTS 5
#define MAX_LEVEL 10
#define CLEANLINESS_DECAY_HOURS 12  
#define HUNGER_DECAY_HOURS 5  
#define HAPPINESS_DECAY_HOURS 8
#define MAX_STAT_LEN 12

//struct for level info
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

//enum type for stats 
typedef enum {
	CLEANLINESS,
	HAPPINESS,
	HUNGER,
} stats;

extern const level levels[MAX_LEVEL - 1];
extern pthread_mutex_t hunger_mutex;
extern pthread_mutex_t cleanliness_mutex;
extern pthread_mutex_t happiness_mutex;

//if hearts in a category are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(int category);

extern void get_stat_string(int category, int amt, char *stat_str);

extern void free_pet(void); 

extern pet new_pet(char *name); 

extern void print_warning(int category);

extern void increase_level( void );

extern void *decrease_cleanliness(void *arg);

extern void *decrease_hunger(void *arg);

extern void *decrease_happiness(void *arg);
