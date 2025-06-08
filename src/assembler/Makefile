CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

OBJS = symtable.o assemble.o aliases.o

all: assemble
	cp assemble ~/armv8_testsuite/solution

assemble: $(OBJS)

assemble.o: assemble.c symtable.h  
symtable.o: symtable.c
aliases.o: aliases.c

clean:
	$(RM) *.o assemble
	$(RM) ../utils/*.o
	
