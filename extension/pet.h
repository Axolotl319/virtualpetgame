struct pet {
	char *name; 
	unsigned int cleanliness;
	unsigned int happiness;
	unsigned int hunger;
};

typedef struct pet *pet;

//if any hearts are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(pet p);

extern void print_hearts(pet p);

extern void free_pet(pet p); 

extern pet new_pet(char *name); 
