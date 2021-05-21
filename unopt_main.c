#include <stdio.h>
#include <stdlib.h>

typedef struct wavContents
{
} wavContents;

int main(int argc, char **argv)
{
	/*
	argc and argv are used for file input, which in the case of this file is a 10-second .wav file
	Thus, argc is also first used for error checking as the program will not run without a .wav file - PSR, 2021-05-20
	*/
	
	if(argc < 2 || argc > 2)
	{
		fprintf(stderr, "Two arguments are required: application declaration and a 10-second .wav file\n");
		exit(1);
	}
	
	/*
	argv[0] - nothing for this function, just the argument to call the application
	argv[1] - the .wav file that is called as the second argument
	
	fopen is used to open the .wav file specified by argv[1] and associate it with a stream accessed by the wavFile pointer
	"rb" - "r" refers to read to allow the file to be opened for input operations, "b" is used for binary since it is a .wav file and not a .txt file
	Personally, I don't think there's a difference between "rb" and "r" but since ARM is something I have no experience with, better to be safe than sorry - PSR, 2021-05-20
	
	Perhaps using "r+" or "rb+" would be better? - PSR, 2021-05-20
	*/
	
	wavFile = fopen(argv[1], "rb");
	
	/*
	Error handling in the event that the file does not work - PSR, 2021-05-20
	*/
	
	if(!wavFile)
	{
		fprintf(stderr, "The input file in the argument is not of a compatible type for the application\n");
		exit(1);
	}
	
	//Start reading the .wav file
	
	//I'm assuming that we would use fread for this, since the data is technically a stream due to fopen()
	
	//Perform mu-law compression - use bit shifts to account for ln(n) = (log2(n))/(log2(e))
	
	//Perform mu-law decompression - reversed process to compression
	
	fclose(wavFile);
	
	return 0;	
}