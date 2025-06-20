CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\

.SUFFIXES: .c .o

.PHONY: all clean

OBJS = pet.o user-actions.o coins.o virtual-pet-exe.o 

all: virtual-text-pet-exe

virtual-text-pet-exe: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

virtual-pet-exe.o: virtual-pet-exe.c user-actions.h pet.h coins.h 
user-actions.o: user-actions.c pet.h coins.h 
pet.o: pet.c coins.h 
coins.o: coins.c

clean:
	$(RM) *.o virtual-text-pet-exe
