struct pet {
	char *name; 
	unsigned int cleanliness;
	unsigned int happiness;
	unsigned int hunger;
};

typedef struct pet *pet;

//global variable - the pet the user will interact with
pet vpet;

//if any hearts are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(void);

extern void print_hearts(void);

extern void free_pet(void); 

extern pet new_pet(char *name); 

