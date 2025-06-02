CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes

.SUFFIXES: .c .o

.PHONY: all clean

all: emulate

emulate: emulate.o sign-extension.o extract-bits.o instr-formats.o data-processing.o data-transfer.o branch.o modify-regs.o

emulate.o: emulate.c armv8.h
data-processing.o: data-processing.c armv8.h
data-transfer.o: data-transfer.c armv8.h
branch.o: branch.c armv8.h
modify-regs.o: modify-regs.c armv8.h

clean:
	$(RM) *.o emulate
	
