CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic -Wstrict-prototypes\
	-I ../utils

.SUFFIXES: .c .o

.PHONY: all clean

UTIL_OBJS := ../utils/instr-formats.o
OBJS = emulate.o data-processing.o data-transfer.o branch.o armv8.o emulator-utils.o $(UTIL_OBJS)

UTIL_HEADERS := ../utils/instr-formats.h ../utils/constants.h emulator-utils.h armv8.h

all: emulate

emulate: $(OBJS)

emulate.o: emulate.c data-processing.h branch.h data-transfer.h $(UTIL_HEADERS)
data-processing.o: data-processing.c $(UTIL_HEADERS) 
data-transfer.o: data-transfer.c $(UTIL_HEADERS)
branch.o: branch.c $(UTIL_HEADERS)
armv8.o: armv8.c ../utils/instr-formats.h
emulator-utils.o: emulator-utils.c ../utils/constants.h

clean:
	$(RM) *.o emulate
	$(RM) ../utils/*.o
	
