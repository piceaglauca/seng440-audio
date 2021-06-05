#include <stdio.h>
#include <stdlib.h>

typedef struct wavContents
{
	/*
	Going by the WAVE Specs .pdf that SH pushed to the GitHub in creating this struct
	The purpose of this struct is for parsing the .wav file that will be uploaded
	The struct is split in pieces, though it could be split into three different structs
	
	This first piece is about the Header of the .wav file that will be uploaded
	The Header has three components: ckID1, cksize1 and WAVEID
	
	ckID1 refers to the Chunk ID, which is 'RIFF', taking up 4 bytes
	cksize1 refers to the overall size of the file and is recorded in bytes - expected to be 4-8 bytes
	WAVEID refers to the file type header and it is a string that markes it as a 'WAVE' - it is 4 bytes - PSR, 2021-05-28	
	*/
	
	unsigned char ckID1[4];
	uint32_t cksize1;
	unsigned char WAVEID[4];
	
	/*
	This second piece is about the format chunk marker of the .wav file that will be uploaded
	The format chunk marker has the components: ckID2, cksize2, wFormatTag, nChannels, nSamplesPerSec, nAvgBytesPerSec, nBlockAlign, wBitsPerSample
	
	ckID2 refers to the CHUNK ID, which is 'fmt', taking up 4 bytes
	cksize2 refers to the overall size of the format data
	wFormatTag refers to the format code, uint16_t is used here since it is clearly smaller
	nChannels refers to the number of interleaved channels, also uint16_t since it is smaller
	nSamplesPerSec refers to the sampling rate (blocks per second)
	nAvgBytesPerSec refers to the data rate
	nBlockAlign refers to the data block size in bytes, it should be 4 bytes
	wBitsPerSample refers to the bits per sample (it is the nBlockAlign / nChannels) - PSR, 2021-05-28
	
	*/
	
	unsigned char ckID2[4];
	uint32_t cksize2;
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint32_t nBlockAlign;
	uint16_t wBitsPerSample;
	
	/*
	This third piece is about the data of the .wav file that will be uploaded
	The data has the components: ckID3, cksize3
	
	ckID3 refers to the data ID, which is 'data', taking up 4 bytes
	cksize3 refers to the overall size of the data
	*/
	
	unsigned char ckID3[4];
	uint32_t cksize3;
	
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
	
	/*
	Start with intializing the object of the struct wavContents to actually hold the parsed data
	In this case, the object will simply be called contents
	size_t result is used for the fread() function as shown here http://www.cplusplus.com/reference/cstdio/fread/
	
	The actual parsing operation will be done with the fread() function since it reads in an array from a stream and stores them 
	
	The first fread operation takes in the value for ckID1, which we know is 'RIFF' 
	
	The second fread operation takes in the buff arrary so it can be used with wavFile 
	After this, contents.cksize1 is defined due to the second fread operations 
	
	Via the third fread operation, WAVEID is given its value, which we know is 'WAVE'
	
	This completes the reading of the 'RIFF' chunk of the .wav file - PSR, 2021-05-20
	*/
	
	wavContents contents;
	unsigned char buff[4];
	size_t result;
	
	result = fread(contents.ckID1, sizeof(contents.ckID1), 1, wavFile);
	
	result = fread(buff, sizeof(uint32_t), 1, wavFile);
	contents.cksize1 = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
	
	result = fread(contents.WAVEID, sizeof(contents.WAVEID), 1 wavFile);
	
	/*
	The next bit of parsing with fread operations will take in values for the format chunk marker
	
	The first fread operation takes in the value for ckID2, which we know is 'fmt'
	
	The second fread operation is a repeat on buff to ensure that it can be used on contents.cksize2
	After this, contents.cksize3 is defined all due to the third operation
	
	The fourth fread operation is a repeat on buff but this time with a smaller size, so sizeof(uint16_t) is used instead
	After this, contents.wFormatTag is defined and it is used to judge the format type of the audio file
	
	The fifth fread operation is a repeat on buff with a smaller size, so sizeof(uint16_t) is once again used
	After this, contents.nChannels is defined due to the fifth operation
	
	The sixth fread operation is a repeat on buff with the normal size for nSamplesPerSec
	After this, contents.nSamplesPerSec is defined due to the sixth operation
	
	The seventh fread operation is a repeat on buff with the normal size for nAvgBytesPerSec
	After this, contents.nAvgBytesPerSec is defined due to the seventh operation
	
	The eighth fread operation is a repeat on buff with the small size for nBlockAlign
	After this, contents.nBlockAlign is defined due to the eighth operation
	
	The ninth fread operation is a repeat on buff with the normal size for wBitsPerSample
	After this, contents.wBitsPerSample is defined due to the eighth operation
	
	This completes the reading of the 'fmt' chunk of the .wav file - PSR, 2021-06-04
	*/
	
	result = fread(contents.ckID2, sizeof(contents.ckID2), 1, wavFile);
	
	result = fread(buff, sizeof(uint32_t), 1, wavFile);
	contents.cksize2 = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
	
	result = fread(buff, sizeof(uint16_t), 1, wavFile);
	contents.wFormatTag = ((buff[0]) | (buff[1] << 8));
	
	result = fread(buff, sizeof(uint16_t), 1, wavFile);
	contents.nChannels = ((buff[0]) | (buff[1] << 8));
	
	result = fread(buff, sizeof(uint32_t), 1, wavFile);
	contents.nSamplesPerSec = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
	
	result = fread(buff, sizeof(uint32_t), 1, wavFile);
	contents.nAvgBytesPerSec = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
	
	result = fread(buff, sizeof(uint16_t), 1, wavFile);
	contents.nBlockAlign = ((buff[0]) | (buff[1] << 8));
	
	result = fread(buff, sizeof(uint16_t), 1, wavFile);
	contents.wBitsPerSample = ((buff[0])) | (buff[1] << 8));
	
	/*
	The next bit of parsing with fread operations will take in values for the data
	
	The first fread operation takes in the value for ckID3, which we know is 'data'
	
	The second fread operation is a repeat on buff with the normal size for cksize3
	After this, contents.cksize3 is defined due to the second operation
	
	This completes the reading of the 'data' chunk of the .wav file - PSR, 2021-06-04
	*/
	
	result = fread(contents.ckID3, sizeof(contents.ckID3), 1, wavFile);
	
	result = fread(buff, sizeof(uint32_t), 1, wavFile);
	contents.cksize3 = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
		
	//Perform mu-law compression - use bit shifts to account for ln(n) = (log2(n))/(log2(e))
	
	//Perform mu-law decompression - reversed process to compression
	
	fclose(wavFile);
	
	return 0;	
}