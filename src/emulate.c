#include <stdlib.h>

#include <stdio.h>

#define WORD_SIZE 4; //1 word = 4 bytes

void decode(char *instruction);

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
	buffer = malloc(MEM_SIZE);
	if(buffer == NULL){
		perror("Couldn't allocate buffer memory");
		fclose(inFile);
		return 1;
	}

	fgets(buffer, MEM_SIZE, inFile);
	fclose(inFile);

	char *current = buffer;
	while(*current != 0){
		decode(current); //function to be written
		current += WORD_SIZE; //next instruction
	}
	free(buffer);
	return EXIT_SUCCESS;
}
