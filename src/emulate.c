#include <stdlib.h>

#include <stdio.h>

#define WORD_SIZE 4; //1 word = 4 bytes

int main(int argc, char **argv) 
{
	//argc = 2 or 3, ./emulate is 1st arg
  	FILE *inFile = fopen(argv[1], "r");
	if(inFile == NULL){
		perror("Couldn't open input file.");
		return 1;
	}
	int MEM_SIZE = 1 << 18;
	char *buffer; //1 char = 1 byte = 8 bits
	fgets(buffer, MEM_SIZE, inFile);
	fclose(inFile);
	while(*buffer != 0){
		decode(buffer); //function to be written
		buffer += WORD_SIZE; //next instruction
	}
	return EXIT_SUCCESS;
}
