#define DEFAULT_CAP 3

struct dyn_array {
  char **data; 
  int size; 
  int cap; 
}; 

typedef struct dyn_array * dyn_array; 

dyn_array create_dyn_array(void); 
void add_elem(dyn_array da, char *); 
void free_dyn_array(dyn_array da); 
