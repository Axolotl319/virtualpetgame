#ifndef LEVELS_H
#define LEVELS_H

#define MAX_LEVEL 10

typedef struct {
	int level_num;
	int num_actions;
} level;

extern const level levels[MAX_LEVEL - 1];

#endif
