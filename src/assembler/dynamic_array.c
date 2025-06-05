#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "dynamic_array.h"

dyn_array create_dyn_array(void) {
  dyn_array new = malloc(sizeof(struct dyn_array));
  assert(new != NULL); 
  new->data = malloc(DEFAULT_CAP * sizeof(char *)); 
  assert(new->data != NULL); 
  new->size = 0; 
  new->cap = DEFAULT_CAP; 
  return new; 
}

static void grow(dyn_array da) {
  da->cap = 2*da->cap; 
  da->data = realloc(da->data, da->cap*sizeof(char *)); 
  assert(da->data != NULL); 
}

void add_elem(dyn_array da, char *d) {
  if (da->size+1 > da->cap) {
    grow(da); 
  }
  da->data[da->size] = d;
  da->size++; 
}

void free_dyn_array(dyn_array da) {
  free(da->data); 
  free(da); 
}

void print_dyn_array(dyn_array da) { 
  if(da->size == 0) {
    printf("[ ]"); 
    return; 
  }
  printf("[ "); 
  for (int i=0; i<da->size-1; i++) {
    printf("%s, ", da->data[i]); 
  }
  printf("%s ]\n", da->data[da->size-1]); 
}

