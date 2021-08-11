#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

/**
 * Data structure to encapsulate characteristics of a WAV file as outlined in
 * the file header. The struct contains a pointer to a int16_t array containing
 * the WAV audio samples in the data chunk. All other values in the data 
 * structure are primitive types.
 * See http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
 */
typedef struct WAVFile
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
     * Otherwise, it can be 18 or 40 bytes for other formats (not supported 
     * here).
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
    
} wav_t;

void readWavFile(char* filename, wav_t * contents) {
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
        fprintf(stderr, "The input file in the argument is not of a compatible "
            "type for the application\n");
        exit(1);
    }
    
    unsigned char buff[4];

    //unsigned char contents->RIFF[4];
    fread(contents->RIFF, sizeof(contents->RIFF), 1, wavFile);
    // can't continue if not RIFF file
    assert (strncmp ((char*) contents->RIFF, "RIFF", 
        sizeof(contents->RIFF)) == 0);
    
    //uint32_t contents->riff_size;
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->riff_size = ((buff[0]) | 
                           (buff[1] <<  8) | 
                           (buff[2] << 16) | 
                           (buff[3] << 24));
    assert (contents->riff_size > 0);
    
    //unsigned char contents->WAVEID[4];
    fread(contents->WAVEID, sizeof(contents->WAVEID), 1, wavFile);
    // can't continue if not WAVE file
    assert (strncmp ((char*) contents->WAVEID, "WAVE", 
        sizeof(contents->WAVEID)) == 0);
    
    //unsigned char contents->fmtID[4];
    fread(contents->fmtID, sizeof(contents->fmtID), 1, wavFile);
    // can't continue without fmt chunk
    assert (strncmp ((char*) contents->fmtID, "fmt ", 
        sizeof(contents->fmtID)) == 0);
    
    //uint32_t contents->fmt_size;
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->fmt_size = ((buff[0]) | 
                          (buff[1] <<  8) | 
                          (buff[2] << 16) | 
                          (buff[3] << 24));
    // if fmt chunk is longer, this is not a valid PCM WAVE file
    assert (contents->fmt_size == 16);
    
    //uint16_t contents->wFormatTag;
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->wFormatTag = ((buff[0]) | (buff[1] << 8));
    assert (contents->wFormatTag == 1); // any other wFormatTag is not PCM WAVE
    
    //uint16_t contents->nChannels;
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->nChannels = ((buff[0]) | (buff[1] << 8));
    assert (contents->nChannels >= 1);
    
    //uint32_t contents->nSamplesPerSec;
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->nSamplesPerSec = ((buff[0]) | 
                                (buff[1] <<  8) | 
                                (buff[2] << 16) | 
                                (buff[3] << 24));
    assert (contents->nSamplesPerSec > 0);
    
    //uint32_t contents->nAvgBytesPerSec;
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->nAvgBytesPerSec = ((buff[0]) | 
                                 (buff[1] <<  8) | 
                                 (buff[2] << 16) | 
                                 (buff[3] << 24));
    assert (contents->nAvgBytesPerSec > 0);
    
    //uint32_t contents->nBlockAlign;
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->nBlockAlign = ((buff[0]) | (buff[1] << 8));
    assert (contents->nBlockAlign > 0);
    
    //uint16_t contents->wBitsPerSample;
    fread(buff, sizeof(u_int16_t), 1, wavFile);
    contents->wBitsPerSample = ((buff[0]) | (buff[1] << 8));
    assert (contents->wBitsPerSample > 0);
    
    /**
     * Some WAVE files contain extra chunks between the 'fmt ' and 'data' 
     * chunks. As a result, we don't know where the data chunk starts. We'll 
     * keep scanning in the file until we find it. It shouldn't be far, since
     * we're still in the file header.
     * This isn't an efficient scanning algorithm, but it's good enough for
     * the expected amount of data in the header that needs to be scanned.
     */
    //unsigned char contents->dataID[4];
    fread(contents->dataID, sizeof(contents->dataID), 1, wavFile);
    while (strncmp ((char*) contents->dataID, "data", 
            sizeof(contents->dataID)) != 0) {
        // Move back "data" - 1 byte
        fseek(wavFile, 1 - sizeof(contents->dataID), SEEK_CUR);
        fread(contents->dataID, sizeof(contents->dataID), 1, wavFile);
    }
    // can't continue without data chunk
    assert (strncmp ((char*) contents->dataID, "data", 
        sizeof(contents->dataID)) == 0);
    
    /** 
     * Read the size of the data chunk, and remember where in the file the data
     * chunk begins.
     */
    //uint32_t contents->data_size;
    fread(buff, sizeof(u_int32_t), 1, wavFile);
    contents->data_size = ((buff[0]) | 
                           (buff[1] <<  8) | 
                           (buff[2] << 16) | 
                           (buff[3] << 24));
    contents->data_offset = ftell(wavFile);
    assert (contents->data_size > 0);
    assert (contents->riff_size - contents->data_size + 8 == 
                    contents->data_offset);

    /**
     * Calculate some details about the file. "8" below is used for the number
     * of bytes comprising the RIFF ckID and cksize
     */
    contents->file_size = 8 + contents->riff_size;
    contents->numSampleFrames = (8 * contents->data_size) / 
                    (contents->nChannels * contents->wBitsPerSample);
    contents->bytesPerSampleFrame = (contents->nChannels * 
                    contents->wBitsPerSample) / 8;

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
 * https://jonathanhays.me/2018/11/14/mu-law-and-a-law-compression-tutorial/
 * Accessed 2021-06-08
 */
const int cBias = 0x84;
const int cClip = 32635;

// 256 bytes (16x16 matrix, 1 byte per element)
static char MuLawCompressTable[256] =
{
    0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

unsigned char CompressSample_LookupTable (int16_t sample) {
    int sign = (sample >> 8) & 0x80;
    if (sign)
        sample = (short)-sample; // absolute value
    if (sample == -32768)
        sample = cClip;
    if (sample > cClip)
        sample = cClip;
    sample = (short)(sample + cBias);
    int exponent = (int) MuLawCompressTable[(sample >> 7) & 0xFF];
    int mantissa = (sample >> (exponent + 3)) & 0x0F;
    int compressedByte = ~ (sign | (exponent << 4) | mantissa);

    return (unsigned char) compressedByte;
}
/** End of borrowed code */

unsigned char CompressSample_Optimized (int16_t sample) {
    register int32_t localSample = (int32_t) sample;
    register u_int32_t sign;
    register int32_t leadingZeroes, exponent, mantissa;
    register unsigned char compressedByte;

    /* If sample was negative, sign = 1. if sample was positive, sign = 0 */
    sign = (localSample >> 15) & 0x1;

    /* Get magnitude (absolute value) from sample */
    localSample = (localSample < 0) ? -localSample : localSample;

    /* Cannot deal with value outside of 16 bits.
     * cClip = (2^16 - 1) - cBias */
    //localSample = (localSample < cClip) ? localSample + cBias : 32767;
    // ^ assembles to branch. Replaced with below.
    localSample += cBias;
    localSample = (localSample < 32767) ? localSample : 32767;

    asm volatile (
        "clz\t%0, %1" // count leading zeroes of sample
        : "=r" (leadingZeroes)
        : "r" (localSample)
    );

    /* Calculate exponent from leading zeroes. Does the equivalent of the
     * lookup table from the unoptimized code. */
    //exponent = (leadingZeroes != 0) ? 24 - leadingZeroes : 0;
    // ^ assembles to branch. Replaced with below.
    exponent = 24 - leadingZeroes;
    exponent = (exponent > 0) ? exponent : 0;

    /* Mantissa is the four bits to the right of the exponent position */
    mantissa = (localSample >> (exponent+3)) & 0xF;

    /* Form compressed byte and cast to unsigned char */
    //compressedByte = (unsigned char) ~ ((sign << 7) | (exponent << 4) | mantissa);
    // ^ assembles to several unnecessary uxth instructions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
    asm volatile (
        "mov\t%0, %1, lsl #7\n\t"
        "orr\t%0, %0, %2, lsl #4\n\t"
        "orr\t%0, %0, %3\n\t"
        "mvn\t%0, %0\n\t"
        "uxtb %0, %0"
        : "+r" (compressedByte)
        : "r" (sign), "r" (exponent), "r" (mantissa)
    );
#pragma GCC diagnostic pop

    return (unsigned char) compressedByte;
}

/*
 * The following implementation was adapted from the project introduction
 * slides given by Dr. Mihai Sima
 */
unsigned char CompressSample_Original (int16_t sample) {
    int32_t exponent, mantissa;
    unsigned char compressedByte;
    int32_t sample_magnitude, sign;

    if ((sample & 0x8000) == 0) {
        sign = 0;
        sample_magnitude = sample;
    } else {
        sign = 1;
        sample_magnitude = -sample; // absolute value
    }

    if (sample == -32768) {
        sample_magnitude = cClip;
    }
    if (sample_magnitude > cClip) {
        sample_magnitude = cClip;
    }
    sample_magnitude = (short) (sample_magnitude + cBias);        

    if (sample_magnitude & (1 << 14)) {
        exponent = 0x7;
        mantissa = (sample_magnitude >> 10) & 0xF;
    } else if (sample_magnitude & (1 << 13)) {
        exponent = 0x6;
        mantissa = (sample_magnitude >> 9) & 0xF;
    } else if (sample_magnitude & (1 << 12)) {
        exponent = 0x5;
        mantissa = (sample_magnitude >> 8) & 0xF;
    } else if (sample_magnitude & (1 << 11)) {
        exponent = 0x4;
        mantissa = (sample_magnitude >> 7) & 0xF;
    } else if (sample_magnitude & (1 << 10)) {
        exponent = 0x3;
        mantissa = (sample_magnitude >> 6) & 0xF;
    } else if (sample_magnitude & (1 << 9)) {
        exponent = 0x2;
        mantissa = (sample_magnitude >> 5) & 0xF;
    } else if (sample_magnitude & (1 << 8)) {
        exponent = 0x1;
        mantissa = (sample_magnitude >> 4) & 0xF;
    } else if (sample_magnitude & (1 << 7)) {
        exponent = 0x0;
        mantissa = (sample_magnitude >> 3) & 0xF;
    } else {
        exponent = 0x0;
        mantissa = 0x0;
    }

    compressedByte = ~((sign << 7) | (exponent << 4) | mantissa);

    return compressedByte;
}
/* End of code adapted from Dr. Mihai Sima */

/**
 * Implementation of mu-law compression algorithm.
 *
 * Input: wave struct, containing 16-bit sound data
 *        filename to output compressed data
 * Output: for benchmarking, time taken to perform compression (using clock() 
 *        and CLOCKS_PER_SEC)
 * Side effect: writes to a file (overwriting if the file exists).
 */
double compress (wav_t* contents, char* filename, unsigned char (*encodefn)(int16_t)) {
    /**
     * Note: if the wBitsPerSample is greater than 16, a short won't
     * be enough space to contain the sample.
     */
    short sample = 0;

    FILE * outfile = fopen (filename, "wb");
    clock_t start_time, end_time;
    /**
     * contents->data_size = the number of bytes in the data chunk. Each 
     * sample frame contains one sample per channel, at wBitsPerSample each.
     * 
     * This for loop iterates over the indices of contents->data, and 
     * increases by the number of bytes per sample. The inner for loop does a
     * bitwise OR to collect the entire sample in a short int. If the audio is
     * mono, there will only be one iteration of the inner for loop. If stereo,
     * only two iterations of the inner for loop.
     */
    start_time = clock();
    for (int i = 0; i < contents->data_size; i+=contents->wBitsPerSample / 8) {
        sample = contents->data[i];
        for (int j = 1; j <= contents->wBitsPerSample / 8; j++) {
            sample |= contents->data[i+j] << (8 * j);
        }
        unsigned char compressed = encodefn (sample);
        fwrite (&compressed, sizeof(compressed), 1, outfile);
    }
    end_time = clock();
    fclose(outfile);

    return (double) ((end_time - start_time) / (1.0 * CLOCKS_PER_SEC));
}

/**
 * The following is taken from 
 * https://jonathanhays.me/2018/11/14/mu-law-and-a-law-compression-tutorial/
 * Accessed 2021-06-08
 */

// 512 bytes (32x8 matrix, 2 bytes per element
static short MuLawDecompressTable[256] =
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
     * Source: https://jonathanhays.me/2018/11/14/mu-law-and-a-law-compression-tutorial/
     */
    while (fread(&compressed, sizeof(compressed), 1, infile) == 1) {
        decompressed = (u_int16_t) MuLawDecompressTable[(int) compressed];
        fwrite(&decompressed, sizeof(decompressed), 1, outfile);
    }
    fclose (outfile);
    fclose (infile);
}
/** End of borrowed code */

int main(int argc, char **argv)
{
    /*
    argc and argv are used for file input, which in the case of this file is a 
    10-second .wav file. Thus, argc is also first used for error checking as 
    the program will not run without a .wav file - PSR, 2021-05-20
    */
    
    if(argc < 2 || argc > 2)
    {
        fprintf(stderr, "Two arguments are required: application declaration "
            "and a 10-second .wav file\n");
        exit(1);
    }
    
    /*
    argv[0] - nothing for this function, just the argument to call the application
    argv[1] - the .wav file that is called as the second argument
    */
    
    /**
     * Contents struct will contain the header and data chunks of the 
     * input WAV file.
     */
    wav_t * contents = (wav_t*) malloc (sizeof(wav_t));
    if (contents == NULL) {
        fprintf (stderr, "malloc failed\n");
        exit(1);
    } else {
        memset (contents, 0, sizeof(wav_t));
    }

    /**
     * Populate contents data structure with values from WAV file header and
     * data
     */
    readWavFile (argv[1], contents);

    /**
     * The pointers to functions are used in compress() to return the time
     * taken by each algorithm.
     */
    unsigned char (*encodefn_original)(int16_t) = CompressSample_Original;
    unsigned char (*encodefn_lookuptable)(int16_t) = CompressSample_LookupTable;
    unsigned char (*encodefn_optimized)(int16_t) = CompressSample_Optimized;

    double timeTaken;

    timeTaken = compress(contents, "test_compress.out", encodefn_original);
    decompress ("test_compress.out", "test_decompress.out");
    printf("Processor time used by the original version of audio compression: "
        "%lf\n", timeTaken);


    timeTaken = compress(contents, "test_compress.out", encodefn_lookuptable);
    decompress ("test_compress.out", "test_decompress.out");
    printf("Processor time used by the lookup table version of audio "
        "compression: %lf\n", timeTaken);


    timeTaken = compress(contents, "test_compress.out", encodefn_optimized);
    decompress ("test_compress.out", "test_decompress.out");
    printf("Processor time used by the assembly optimized version of audio "
        "compression: %lf\n", timeTaken);


    if (contents->data != NULL) {
        free(contents->data);
    }
    free (contents);
    
    return 0;    
}
