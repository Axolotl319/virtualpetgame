CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

UTIL_OBJS := ../utils/sign-extension.o ../utils/extract-data.o ../utils/instr-formats.o
OBJS = emulate.o data-processing.o data-transfer.o branch.o modify-regs.o $(UTIL_OBJS)

all: emulate
	cp emulate ~/armv8_testsuite/solution

emulate: $(OBJS)

emulate.o: emulate.c armv8.h
data-processing.o: data-processing.c armv8.h
data-transfer.o: data-transfer.c armv8.h
branch.o: branch.c armv8.h
modify-regs.o: modify-regs.c armv8.h

clean:
	$(RM) *.o emulate
	$(RM) ../utils/*.o
	
