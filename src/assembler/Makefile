CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes

.SUFFIXES: .c .o

.PHONY: all clean

OBJS = symtable.o assemble.o dynamic_array.o

all: assemble

assemble: $(OBJS)

assemble.o: assemble.c symtable.h dynamic_array.h
symtable.o: symtable.c
dyn_array.o: dynamic_array.c

clean:
	$(RM) *.o assemble
	
