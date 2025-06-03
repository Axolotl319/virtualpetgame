all:
	+$(MAKE) -C assembler
	+$(MAKE) -C emulator
	mv emulator/emulate .
	mv assembler/assemble .

clean:
	+$(MAKE) -C assembler clean
	+$(MAKE) -C emulator clean
	$(RM) *.o assemble emulate
	
