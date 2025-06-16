CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

OBJS = pet.o user-actions.o virtual-pet-exe.o

all: virtual-pet-exe

virtual-pet-exe: $(OBJS)

user-actions.o: user-actions.c
pet.o: pet.c
