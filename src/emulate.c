#include <stdlib.h>
#include <stdio.h>

const int WORD_SIZE = 4; 	// 1 word = 4 bytes
const int MEM_SIZE = 1 << 21; 	// ARMv8 has 2MB memory 

// TODO -- the execution 
void immdp(int instr) {} 
void regdp(int instr) {}
void loadliteral(int instr) {} 
void datatransfer(int instr) {}
void branch(int instr) {}

// Given the starting address of an instruction, determines the instruction type 
// and passes the instruction to the corresponding function to handle. 
// Returns 1 if decoding unsuccessful, returns 0 if successful.  
int decode(char *instruction) {
	
	// Combines four consecutive bytes to 32 bits, taking into account little endian  
	int result = ((0xff & *(instruction+3)) << 24) 
		| ((0xff & *(instruction+2)) << 16) 
		| ((0xff & *(instruction+1)) << 8) 
		| ((0xff & *instruction));

	// Obtain op0 -- comments used for debugging purposes. 
	int opzero = (result >> 25) & 0xf;
        printf("%u ", opzero);	
	if (opzero == 8 || opzero == 9) {
		printf("This is data processing (immediate).\n");
		immdp( result ); 
	} else if (opzero == 5 || opzero == 13) {
		printf("This is data processing (registers).\n");
		regdp( result ); 
	} else if (opzero == 4 || opzero == 6 || opzero == 12 || opzero == 14) {
		printf("This is single data transfer.\n"); 
		int temp = 0xf & (result >> 31); 
		if (temp) {
			datatransfer( result ); 
		} else {
			loadliteral( result ); 
		}
	} else if (opzero == 10 || opzero == 11) {
		printf("This is branch.\n"); 
		branch( result ); 
	} else {
		perror("Bad opcode (op0).\n");
		return 1; 
	}
	
	return 0; 
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
		if ( decode( current ) ) {
			return 1; 
		}	
		current += WORD_SIZE; 
	}	
	
	free(buffer);
	return EXIT_SUCCESS;
}
