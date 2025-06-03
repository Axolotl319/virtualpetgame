CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble

assemble: $(OBJS)

assemble.o: assemble.c
symtable.o: symtable.c

clean:
	$(RM) *.o assemble
	
