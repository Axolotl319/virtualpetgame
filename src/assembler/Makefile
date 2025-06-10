CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

UTIL_OBJS = ../utils/tokenise_params.o

OBJS = symtable.o assemble.o aliases.o branch.o data-transfer.o data-processing.o $(UTIL_OBJS)

all: assemble
	cp assemble ~/armv8_testsuite/solution

assemble: $(OBJS) 

assemble.o: assemble.c symtable.h  
symtable.o: symtable.c
aliases.o: aliases.c
data-processing.o: data-processing.c
data-transfer.o: data-transfer.c
branch.o: branch.c

clean:
	$(RM) *.o assemble
	$(RM) ../utils/*.o
	
