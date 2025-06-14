struct hearts{
	unsigned int cleanliness;
	unsigned int happiness;
	unsigned int hunger;
};

typedef struct hearts *hearts;

//if any hearts are set to > 5 or < 0
//check_bounds changes them to 0/5
extern void check_bounds(hearts h);

extern void print_hearts(hearts h);

extern hearts new_pet(void);

extern void free_hearts(hearts h);
