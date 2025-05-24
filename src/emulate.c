#include <stdlib.h>

#include <stdio.h>

#define WORD_SIZE 4; // 1 word = 4 bytes
const int MEM_SIZE = 1 << 21; // ARMv8 has 2MB memory 

void decode(char *instruction) {

}

int main(int argc, char **argv) 
{
	//argc = 2 or 3, ./emulate is 1st arg
  	FILE *inFile = fopen(argv[1], "rb");
	FILE *outFile;
	if(inFile == NULL){
		perror("Couldn't open input file.");
		return 1;
	}

	if(argc >= 3){
		outFile = fopen(argv[2], "w");
	}else{
		outFile = stdout;
	}


	char *buffer; //1 char = 1 byte = 8 bits
	buffer = malloc(MEM_SIZE);
	if(buffer == NULL){
		perror("Couldn't allocate buffer memory");
		fclose(inFile);
		return 1;
	}

	// Obtains size of file 
	fseek(inFile, 0, SEEK_END); 
	int filesize = ftell(inFile); 
	rewind(inFile);
	
	// Reads contents of file into buffer 
	fgets(buffer, MEM_SIZE, inFile);	
	fclose(inFile);
	
	// Decodes each instruction 
	for(int i=0; i<filesize; i+=4) {
		decode( &buffer[i] );
	        printf("%02x ", buffer[i]); 	
	}

	/*
	char *current = buffer;	

	while(*current != 0){ 
		decode(current); //function to be written
		printf("%02x ", *current); 
		current += WORD_SIZE; //next instruction
	}
	*/

	printf("\n"); 
	free(buffer);
	return EXIT_SUCCESS;
}
