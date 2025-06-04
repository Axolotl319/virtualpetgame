CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

UTIL_OBJS = ../utils/sign-extension.o
OBJS = assemble.o symtable.o $(UTIL_OBJS)

all: assemble

assemble: $(OBJS)

assemble.o: assemble.c
symtable.o: symtable.c

clean:
	$(RM) *.o assemble
	$(RM) ../utils/*.o
	
