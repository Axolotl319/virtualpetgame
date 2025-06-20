CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

UTIL_OBJS = ../utils/instr-formats.o

OBJS = symtable.o assemble.o aliases.o int-directives.o branch.o data-transfer.o data-processing.o assembly-utils.o $(UTIL_OBJS)

UTIL_HEADERS = ../utils/instr-formats.h ../utils/constants.h assembly-utils.h symtable.h aliases.h
INSTR_HEADERS = int-directives.h branch.h data-processing.h data-transfer.h


all: assemble

assemble: $(OBJS) 

assemble.o: assemble.c $(UTIL_HEADERS) $(INSTR_HEADERS)
symtable.o: symtable.c
aliases.o: aliases.c $(INSTR_HEADERS)
data-processing.o: data-processing.c $(UTIL_HEADERS)
data-transfer.o: data-transfer.c $(UTIL_HEADERS)
branch.o: branch.c $(UTIL_HEADERS)
assembly-utils.o: assembly-utils.c
int-directives.o: int-directives.c $(UTIL_HEADERS)

clean:
	$(RM) *.o assemble
	$(RM) ../utils/*.o
	
