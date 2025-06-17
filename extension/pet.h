struct pet {
	char *name; 
	unsigned int cleanliness;
	unsigned int happiness;
	unsigned int hunger;
	bool alive;
};

typedef struct pet *pet;

#ifndef VPET_GLOBAL
#define VPET_GLOBAL
//global variable - the pet the user will interact with
extern pet vpet;
#endif

//if any hearts are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(void);

extern void print_hearts(void);

extern void free_pet(void); 

extern pet new_pet(char *name); 

extern void print_warning(int category);
