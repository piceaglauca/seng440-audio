#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

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
    
    fread(contents->RIFF, sizeof(contents->RIFF), 1, wavFile);
    assert (strncmp ((char*) contents->RIFF, "RIFF", sizeof(contents->RIFF)) == 0); // can't continue if not RIFF file
    
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->riff_size = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->riff_size > 0);
    
    fread(contents->WAVEID, sizeof(contents->WAVEID), 1, wavFile);
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
    
    fread(contents->fmtID, sizeof(contents->fmtID), 1, wavFile);
    assert (strncmp ((char*) contents->fmtID, "fmt ", sizeof(contents->fmtID)) == 0); // can't continue without fmt chunk
    
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->fmt_size = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->fmt_size == 16); // if fmt chunk is longer, this is not a valid PCM WAVE file
    
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->wFormatTag = ((buff[0]) | (buff[1] << 8));
    assert (contents->wFormatTag == 1); // any other wFormatTag is not PCM WAVE
    
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->nChannels = ((buff[0]) | (buff[1] << 8));
    assert (contents->nChannels >= 1);
    
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->nSamplesPerSec = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->nSamplesPerSec > 0);
    
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->nAvgBytesPerSec = ((buff[0]) | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
    assert (contents->nAvgBytesPerSec > 0);
    
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->nBlockAlign = ((buff[0]) | (buff[1] << 8));
    assert (contents->nBlockAlign > 0);
    
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->wBitsPerSample = ((buff[0]) | (buff[1] << 8));
    assert (contents->wBitsPerSample > 0);
    
    /**
     * Some WAVE files contain extra chunks between the 'fmt ' and 'data' 
     * chunks. As a result, we don't know where the data chunk starts. We'll 
     * keep scanning in the file until we find it. It shouldn't be far, since
     * we're still in the file header.
     */
    fread(contents->dataID, sizeof(contents->dataID), 1, wavFile);
    while (strncmp ((char*) contents->dataID, "data", sizeof(contents->dataID)) != 0) {
        // Move back "data" - 1 byte
        fseek(wavFile, 1 - sizeof(contents->dataID), SEEK_CUR);
        fread(contents->dataID, sizeof(contents->dataID), 1, wavFile);
    }
    assert (strncmp ((char*) contents->dataID, "data", sizeof(contents->dataID)) == 0); // can't continue without data chunk
    
    // Read the size of the data chunk, and remember the offset
    fread(buff, sizeof(u_int32_t), 1, wavFile);
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

/**
 * The following is taken from 
 * https://web.archive.org/web/20110719132013/http://hazelware.luggle.com/tutorials/mulawcompression.html
 * Author unknown. Original host site is no longer available.
 * Accessed 2021-06-08
 */
const int32_t cBias = 0x84;
const int32_t cClip = 32635;

// 512 bytes (32x8 matrix, 2 bytes per element
static int16_t MuLawDecompressTable[256] =
{
    -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
    -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
    -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
    -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
     -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
     -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
     -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
     -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
     -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
     -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
      -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
      -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
      -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
      -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
      -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
       -56,   -48,   -40,   -32,   -24,   -16,    -8,    -1,
     32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
     23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
     15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
     11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
      7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
      5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
      3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
      2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
      1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
      1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
       876,   844,   812,   780,   748,   716,   684,   652,
       620,   588,   556,   524,   492,   460,   428,   396,
       372,   356,   340,   324,   308,   292,   276,   260,
       244,   228,   212,   196,   180,   164,   148,   132,
       120,   112,   104,    96,    88,    80,    72,    64,
        56,    48,    40,    32,    24,    16,     8,     0
};

unsigned char LinearToMuLawSample(int16_t sample)
{
    /*
     * Inserts a marker into the assembly. 
     * Search for regex: ^@[^"]*"opt_main.c
     */
    asm("nop");

    int32_t sign = sample & 0x8000;
    if (sign) {
        sample = (int16_t)-sample;
    }
    if (sample == -32768)
        sample = 32767;
    if (sample > cClip) // 32635
        sample = cClip;
    sample = (int16_t)(sample + cBias); // 0x84 = 132

    int32_t leadingZeroes;
    asm volatile (
        "clz\t%0, %1\n"
        : "=r" (leadingZeroes)
        : "r"  (sample)
    );
    int32_t exponent = 24 - leadingZeroes; // this does the equivalent of the lookup table
    int32_t mantissa = (sample >> (exponent+3)) & 0xF;
    unsigned char compressedByte = ~ ((sign >> 8) | (exponent << 4) | mantissa);

    /*
     * Inserts a marker into the assembly. 
     * Search for regex: ^@[^"]*"opt_main.c
     */
    asm("nop");
    return compressedByte;
}
/** End of borrowed code */

/*
The code containing the following implementation of the logarithmic compression function will go here:

		y =  ln(1 + mu|x|)
			--------------
			  ln(1 + mu)

			  
*/
void compressionFunction()
{
	//First calculate the numerator
	//Then calculate the denominator
}

/**
 * Implementation of mu-law compression algorithm.
 *
 * Input: wave struct, containing 16-bit sound data
 *        filename to output compressed data
 * Output: none
 * Side effect: writes to a file (overwriting if the file exists).
 */
void compress (wave * contents, char * filename) {
    /**
     * Note: if the wBitsPerSample is greater than 16, a short won't
     * be enough space to contain the sample.
     */
    int16_t sample = 0;

    FILE * outfile = fopen (filename, "wb");
    /**
     * contents->data_size = the number of bytes in the data chunk. Each 
     * sample frame contains one sample per channel, at wBitsPerSample each.
     * 
     * This for loop iterates over the indices of contents->data, and 
     * increases by the number of bytes per sample. The inner for loop does a
     * bitwise OR to collect the entire sample in a short int.
     */
    for (int i = 0; i < contents->data_size; i+=contents->wBitsPerSample / 8) {
        sample = contents->data[i];
        for (int j = 1; j <= contents->wBitsPerSample / 8; j++) {
            sample |= contents->data[i+j] << (8 * j);
        }
        unsigned char compressed = LinearToMuLawSample (sample);
        fwrite (&compressed, sizeof(compressed), 1, outfile);
    }
    fclose(outfile);
}

/**
 * Implementation of mu-law decompression algorithm.
 *
 * Input: filename to input compressed data from
 *        filename to output decompressed data
 * Output: none
 * Side effect: writes to a file (overwriting if the file exists).
 */
void decompress (char * infilename, char * outfilename) {
    unsigned char compressed = 0;
    u_int16_t decompressed = 0;

    FILE * infile = fopen (infilename, "rb");
    FILE * outfile = fopen (outfilename, "wb");
    /**
     * For each compressed sample in the input file, decompress it, and write
     * the result to the output file.
     * Note: This implementation of decompression uses a 512-byte lookup table
     * to decompress the 8-bit sample into 16-bits. The index of the value is
     * simply the 8-bit compressed codeword. The output will be the 
     * corresponding 16-bit decompressed value.
     * Source: https://web.archive.org/web/20110719132013/http://hazelware.luggle.com/tutorials/mulawcompression.html
     */
    while (fread(&compressed, sizeof(compressed), 1, infile) == 1) {
        decompressed = (u_int16_t) MuLawDecompressTable[(int)compressed];
        fwrite(&decompressed, sizeof(decompressed), 1, outfile);
    }
    fclose (outfile);
    fclose (infile);
}

int main(int argc, char **argv)
{
	/*
	The function clock() is used to return the number of clock ticks elapsed since the program was launched.
	Given that this is the start of the program, startTime is defined and initialized here. - PSR, 2021-07-13
	*/
	clock_t startTime, endTime;
	startTime = clock();
	
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

    printf ("%20s: %8hd %s\n", "nChannels", contents->nChannels, "from header");
    printf ("%20s: %8d %s\n", "nSamplesPerSec", contents->nSamplesPerSec, "from header");
    printf ("%20s: %8d %s\n", "nAvgBytesPerSec", contents->nAvgBytesPerSec, "from header");
    printf ("%20s: %8d %s\n", "nBlockAlign", contents->nBlockAlign, "from header");
    printf ("%20s: %8hd %s\n", "wBitsPerSample", contents->wBitsPerSample, "from header");
    printf ("%20s: %8d %s\n", "data_size", contents->data_size, "from data chksize");
    printf ("%20s: %8d %s\n", "riff_size", contents->riff_size, "from riff chksize");
    printf ("%20s: %8d %s\n", "file_size", contents->file_size, "calc from 8+riff_size");
    printf ("%20s: %8d %s\n", "data_offset", contents->data_offset, "found in file scan");
    printf ("%20s: %8d %s\n", "numSampleFrames", contents->numSampleFrames, 
            "calc from (8*data_size)/(nChannels * wBitsPerSample)");
    printf ("%20s: %8hd %s\n", "bytesPerSampleFrame", contents->bytesPerSampleFrame, 
            "calc from (nChannels * wBitsPerSample) / 8");
    compress(contents, "test_compress.out");
    decompress ("test_compress.out", "test_decompress.out");

    if (contents->data != NULL) {
        free(contents->data);
    }
    free (contents);
	
	/*
	To get the performance, endTime is finally defined here. But since it is in clock ticks, it is converted into a value in seconds.
	This conversion is done using the macro CLOCKS_PER_SEC which is provided by the time.h library.
	It is casted to double for precision to represent the value in seconds, as the difference in the optimized program may be that - PSR, 2021-07-13
	*/
	
	endTime = clock();
	printf("Processor time used by the unoptimized audio compression and decompression program: %lf\n", (double) ((endTime - startTime) / CLOCKS_PER_SEC));
    
    return 0;    
}
