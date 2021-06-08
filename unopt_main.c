#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct wave
{
    /** 
     * Chunk ID: "RIFF" 
     * Note: we probably don't need to store this.
     */
	unsigned char RIFF[4];

    /**
     * RIFF size is the number of bytes following the size field.
     * Total file size is 8 + riff_size. 8 bytes from the ID and size fields.
     */
	u_int32_t riff_size; 

    /**
     * Chunk ID: "WAVE"
     * Note: we probably don't need to store this.
     */
	unsigned char WAVEID[4];
	
    /**
     * Chunk ID: "fmt " (space is intentional, to make field 4 bytes)
     * Note: we probably don't need to store this.
     */
	unsigned char fmtID[4];

    /**
     * Size of the fmt chunk. Should be 16 bytes for an uncompressed PCM file.
     * Otherwise, it can be 18 or 40 bytes for other formats (not supported here).
     */
	u_int32_t fmt_size;

    /**
     * WAVE format code:
     * 0x0001 WAVE_FORMAT_PCM (only format supported here)
     * 0x0003 WAVE_FORMAT_IEEE_FLOAT
     * 0x0006 WAVE_FORMAT_ALAW
     * 0x0007 WAVE_FORMAT_MULAW
     * 0xFFFE WAVE_FORMAT_EXTENSIVE
     */
	u_int16_t wFormatTag;
    
    /**
     * Number of channels in a sample frame. Can be 1 or 2.
     * Additional channels require an unsupported format.
     */
	u_int16_t nChannels;

    /**
     * Audio bitrate, in Hertz. The number of sample frames per second.
     */
	u_int32_t nSamplesPerSec;

    /**
     * = nSamplesPerSec * wBitsPerSample
     */
	u_int32_t nAvgBytesPerSec;

    /**
     * Number of bytes used to store a sample frame. All channels are contained
     * within a frame.
     */
	u_int32_t nBlockAlign;

    /**
     * Number of bits used to store a single sample, where a sample is a single
     * channel.
     */
	u_int16_t wBitsPerSample;
	
	/*
	This third piece is about the data of the .wav file that will be uploaded
	The data has the components: dataID, data_size
	
	dataID refers to the data ID, which is 'data', taking up 4 bytes
	data_size refers to the overall size of the data
	*/
	
    /**
     * Chunk ID: "data"
     * Note: we probably don't need to store this.
     */
	unsigned char dataID[4];

    /**
     * Size of the data chunk, in bytes.
     */
	u_int32_t data_size;

    /**
     * Data array for each sample following the data chunk ID and size.
     * Size of array will be given in the chunk size (data_size).
     */
    unsigned char * data; // PCM data

    /**
     * File offset (from beginning) to where the data chunk contents starts.
     * This position is after the chunk ID and chunk size. Seeking to this 
     * location will put the file pointer at actual audio data.
     */
    u_int32_t data_offset;

    /**
     * File size calculated from 8+riff_size.
     * Note: we probably don't need to store this. Storing one of riff_size or
     * file_size may be important, but both are not necessary.
     */
    u_int32_t file_size;

    /**
     * Number of sample frames, where a frame contains one sample from each
     * channel.
     * Calculated from: (8 * data_size) / (nChannels * wBitsPerSample);
     */
    u_int32_t numSampleFrames;

    /**
     * Number of bytes per sample frame, where a frame contains one sample for
     * each channel.
     * Calculated from: (nChannels * wBitsPerSample) / 8;
     */
    u_int16_t bytesPerSampleFrame;
	
} wave;

void checkTestFile(wave *contents) {
	//unsigned char RIFF[4];
	assert (strncmp ((char*)contents->RIFF, "RIFF", sizeof(contents->RIFF)) == 0);

	//uint32_t riff_size;
	assert (contents->riff_size == 48734362);

	//unsigned char WAVEID[4];
	assert (strncmp ((char*) contents->WAVEID, "WAVE", sizeof(contents->WAVEID)) == 0);
	
	/*
	This second piece is about the format chunk marker of the .wav file that will be uploaded
	The format chunk marker has the components: fmtID, fmt_size, wFormatTag, nChannels, nSamplesPerSec, nAvgBytesPerSec, nBlockAlign, wBitsPerSample
	
	fmtID refers to the CHUNK ID, which is 'fmt', taking up 4 bytes
	fmt_size refers to the overall size of the format data
	wFormatTag refers to the format code, uint16_t is used here since it is clearly smaller
	nChannels refers to the number of interleaved channels, also uint16_t since it is smaller
	nSamplesPerSec refers to the sampling rate (blocks per second)
	nAvgBytesPerSec refers to the data rate
	nBlockAlign refers to the data block size in bytes, it should be 4 bytes
	wBitsPerSample refers to the bits per sample (it is the nBlockAlign / nChannels) - PSR, 2021-05-28
	
	*/
	
	//unsigned char fmtID[4];
    assert (strncmp ((char*) contents->fmtID, "fmt ", sizeof(contents->fmtID)) == 0);

	//uint32_t fmt_size;
    assert (contents->fmt_size == 16);

	//uint16_t wFormatTag;
    assert (contents->wFormatTag == 1);

	//uint16_t nChannels;
    assert (contents->nChannels == 2);
    
	//uint32_t nSamplesPerSec;
    assert (contents->nSamplesPerSec == 44100);

	//uint32_t nAvgBytesPerSec;
    assert (contents->nAvgBytesPerSec == 176400);

	//uint32_t nBlockAlign;
    assert (contents->nBlockAlign == 4);

	//uint16_t wBitsPerSample;
    assert (contents->wBitsPerSample == 16);
	
	/*
	This third piece is about the data of the .wav file that will be uploaded
	The data has the components: dataID, data_size
	
	dataID refers to the data ID, which is 'data', taking up 4 bytes
	data_size refers to the overall size of the data
	*/
	
	//unsigned char dataID[4];
    assert (strncmp ((char*) contents->dataID, "data", sizeof(contents->dataID)) == 0);

	//uint32_t data_size;
    assert (contents->data_size == 48734208);
}

void readWavFile(char* filename, wave * contents) {
    /**
	 * fopen is used to open the .wav file specified by argv[1] and associate 
     * it with a stream accessed by the wavFile pointer "rb" - "r" refers to 
     * read to allow the file to be opened for input operations, "b" is used 
     * for binary since it is a .wav file and not a .txt file.
     *
     * Personally, I don't think there's a difference between "rb" and "r" but 
     * since ARM is something I have no experience with, better to be safe than 
     * sorry - PSR, 2021-05-20
     * Perhaps using "r+" or "rb+" would be better? - PSR, 2021-05-20
	*/
	FILE * wavFile = fopen(filename, "rb");
	
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
	Start with intializing the object of the struct wave to actually hold the parsed data
	In this case, the object will simply be called contents
	size_t result is used for the fread() function as shown here http://www.cplusplus.com/reference/cstdio/fread/
	
	The actual parsing operation will be done with the fread() function since it reads in an array from a stream and stores them 
	
	The first fread operation takes in the value for RIFF, which we know is 'RIFF' 
	
	The second fread operation takes in the buff arrary so it can be used with wavFile 
	After this, contents->riff_size is defined due to the second fread operations 
	
	Via the third fread operation, WAVEID is given its value, which we know is 'WAVE'
	
	This completes the reading of the 'RIFF' chunk of the .wav file - PSR, 2021-05-20
	*/
	
	unsigned char buff[4];
	size_t result;
	
	result = fread(contents->RIFF, sizeof(contents->RIFF), 1, wavFile);
    assert (strncmp ((char*) contents->RIFF, "RIFF", sizeof(contents->RIFF)) == 0); // can't continue if not RIFF file
	
	result = fread(buff, sizeof(u_int32_t), 1, wavFile);
	contents->riff_size = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->riff_size > 0);
	
	result = fread(contents->WAVEID, sizeof(contents->WAVEID), 1, wavFile);
    assert (strncmp ((char*) contents->WAVEID, "WAVE", sizeof(contents->WAVEID)) == 0); // can't continue if not WAVE file
	
	/*
	The next bit of parsing with fread operations will take in values for the format chunk marker
	
	The first fread operation takes in the value for fmtID, which we know is 'fmt'
	
	The second fread operation is a repeat on buff to ensure that it can be used on contents->fmt_size
	After this, contents->data_size is defined all due to the third operation
	
	The fourth fread operation is a repeat on buff but this time with a smaller size, so sizeof(uint16_t) is used instead
	After this, contents->wFormatTag is defined and it is used to judge the format type of the audio file
	
	The fifth fread operation is a repeat on buff with a smaller size, so sizeof(uint16_t) is once again used
	After this, contents->nChannels is defined due to the fifth operation
	
	The sixth fread operation is a repeat on buff with the normal size for nSamplesPerSec
	After this, contents->nSamplesPerSec is defined due to the sixth operation
	
	The seventh fread operation is a repeat on buff with the normal size for nAvgBytesPerSec
	After this, contents->nAvgBytesPerSec is defined due to the seventh operation
	
	The eighth fread operation is a repeat on buff with the small size for nBlockAlign
	After this, contents->nBlockAlign is defined due to the eighth operation
	
	The ninth fread operation is a repeat on buff with the normal size for wBitsPerSample
	After this, contents->wBitsPerSample is defined due to the eighth operation
	
	This completes the reading of the 'fmt' chunk of the .wav file - PSR, 2021-06-04
	*/
	
	result = fread(contents->fmtID, sizeof(contents->fmtID), 1, wavFile);
    assert (strncmp ((char*) contents->fmtID, "fmt ", sizeof(contents->fmtID)) == 0); // can't continue without fmt chunk
	
	result = fread(buff, sizeof(u_int32_t), 1, wavFile);
	contents->fmt_size = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->fmt_size == 16); // if fmt chunk is longer, this is not a valid PCM WAVE file
	
	result = fread(buff, sizeof(u_int16_t), 1, wavFile);
	contents->wFormatTag = ((buff[0]) | (buff[1] << 8));
    assert (contents->wFormatTag == 1); // any other wFormatTag is not PCM WAVE
	
	result = fread(buff, sizeof(u_int16_t), 1, wavFile);
	contents->nChannels = ((buff[0]) | (buff[1] << 8));
    assert (contents->nChannels >= 1);
	
	result = fread(buff, sizeof(u_int32_t), 1, wavFile);
	contents->nSamplesPerSec = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->nSamplesPerSec > 0);
	
	result = fread(buff, sizeof(u_int32_t), 1, wavFile);
	contents->nAvgBytesPerSec = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->nAvgBytesPerSec > 0);
	
	result = fread(buff, sizeof(u_int16_t), 1, wavFile);
	contents->nBlockAlign = ((buff[0]) | (buff[1] << 8));
    assert (contents->nBlockAlign > 0);
	
	result = fread(buff, sizeof(u_int16_t), 1, wavFile);
	contents->wBitsPerSample = ((buff[0]) | (buff[1] << 8));
    assert (contents->wBitsPerSample > 0);
	
    /**
     * Some WAVE files contain extra chunks between the 'fmt ' and 'data' 
     * chunks. As a result, we don't know where the data chunk starts. We'll 
     * keep scanning in the file until we find it. It shouldn't be far, since
     * we're still in the file header.
     */
	result = fread(contents->dataID, sizeof(contents->dataID), 1, wavFile);
    while (strncmp ((char*) contents->dataID, "data", sizeof(contents->dataID)) != 0) {
        // Move back "data" - 1 byte
        fseek(wavFile, 1 - sizeof(contents->dataID), SEEK_CUR);
        result = fread(contents->dataID, sizeof(contents->dataID), 1, wavFile);
    }
    assert (strncmp ((char*) contents->dataID, "data", sizeof(contents->dataID)) == 0); // can't continue without data chunk
	
    // Read the size of the data chunk, and remember the offset
	result = fread(buff, sizeof(u_int32_t), 1, wavFile);
	contents->data_size = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    contents->data_offset = ftell(wavFile);
    assert (contents->data_size > 0);
    assert (contents->riff_size - contents->data_size + 8 == contents->data_offset);

    // Calculate some details about the file
    contents->file_size = 8 + contents->riff_size; // 8 bytes for the RIFF ckID and cksize
    contents->numSampleFrames = (8 * contents->data_size) / (contents->nChannels * contents->wBitsPerSample);
    contents->bytesPerSampleFrame = (contents->nChannels * contents->wBitsPerSample) / 8;

    /**
     * Read the PCM data from the WAVE data chunk
     */
    contents->data = (unsigned char*) malloc (contents->data_size);
    if (contents->data == NULL) {
        fprintf (stderr, "malloc failed\n");
        exit(1);
    } else {
        memset (contents->data, 0, contents->data_size);
    }
    fread (contents->data, contents->data_size, 1, wavFile);
	
	fclose(wavFile);
}

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
	*/
	
	wave * contents = (wave*) malloc (sizeof(wave));
    if (contents == NULL) {
        fprintf (stderr, "malloc failed\n");
        exit(1);
    } else {
        memset (contents, 0, sizeof(wave));
    }

    readWavFile (argv[1], contents);
    checkTestFile(contents);

    if (contents->data != NULL) {
        free(contents->data);
    }
    free (contents);
	
	return 0;	
}
