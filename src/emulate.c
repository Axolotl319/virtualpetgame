#include <stdlib.h>

#include <stdio.h>

const int WORD_SIZE = 4; 	// 1 word = 4 bytes
const int MEM_SIZE = 1 << 21; 	// ARMv8 has 2MB memory 

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

	// Obtains size of file, kept in case needed later 
	// fseek(inFile, 0, SEEK_END); 
	// int filesize = ftell(inFile); 
	// rewind(inFile);
	
	// Reads contents of file into buffer 
	fgets(buffer, MEM_SIZE, inFile);	
	fclose(inFile);
		
	// Decodes each instruction 
	char *current = buffer; 
	while( (*(current+WORD_SIZE-1) & 0xff) != 0x8a ) {
		decode( current ); 
		printf( "%02x ", *current ); 
		current += WORD_SIZE; 
	}	
	
	printf("\n"); 
	free(buffer);
	return EXIT_SUCCESS;
}
