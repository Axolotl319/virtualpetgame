#define INIT_HEARTS 5
#define MAX_LEVEL 10

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

//if any hearts are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(void);

extern void print_hearts(void);

extern void free_pet(void); 

extern pet new_pet(char *name); 

extern void print_warning(int category);

extern void increase_level( void );

