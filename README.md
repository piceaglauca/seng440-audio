# SENG 440 Audio Compression

Final project for SENG 440: Embedded Systems.  
Implement and optimize an implementation of mu-law audio compression.  

Team 17:  
Paramvir Randhawa - V00848357  
Scott Howard - V00847529

https://github.com/piceaglauca/seng440-audio.git

## Prerequisites

GCC version 8.2.1 for ARMv7  
    ```dnf install gcc-8.2.1-2.fc29.armv7hl```  
make version 4.2.1 for ARMv7  
    ```dnf install make-1:4.2.1-10.fc29.armv7hl```

Optional: alsa-utils version 1.1.6 (provides aplay, to test program output)  
    ```dnf install alsa-utils-1.1.6-5.fc29.armv7hl```

## Compilation Instructions

To compile into a binary and assembly, just run ```make```:

```
~/seng440-audio# make
armv7hl-redhat-linux-gnueabi-gcc -Wall -O3 -g -o mulaw-compress mulaw-compress.c
armv7hl-redhat-linux-gnueabi-gcc -Wall -O3 -S -o mulaw-compress.s mulaw-compress.c
```

## Testing Compression

To run the compression algorithms on a test file use ```make test```:

```
~/seng440-audio# make test
./mulaw-compress test_file.wav
Processor time used by the original version of audio compression: 7.530905
Processor time used by the lookup table version of audio compression: 7.127602
Processor time used by the assembly optimized version of audio compression: 6.930987
```

The result will be a ```test_compress.out``` and ```test_decompress.out``` file, containing
the compressed and decompressed data chunk of the input WAV file. The output files do not contain
valid WAV headers, so playing them requires telling the player what to expect from the file.

```
seng440-audio# cat test_compress.out| aplay -c 2 -r 44100 -t raw -f MU_LAW
Playing raw data 'stdin' : Mu-Law, Rate 44100 Hz, Stereo...

seng440-audio# cat test_decompress.out| aplay -c 2 -r 44100 -t raw -f S16_LE
Playing raw data 'stdin' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo...
```
