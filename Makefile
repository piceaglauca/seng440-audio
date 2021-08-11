CC=armv7hl-redhat-linux-gnueabi-gcc
CFLAGS=-Wall -O3

SRC_FILE=mulaw-compress.c
ASM_FILE=mulaw-compress.s
OUT_FILE=mulaw-compress

OUTFILES=$(ASM_FILE) $(OUT_FILE)

.PHONY: clean test all

all: $(OUT_FILE) $(ASM_FILE)

$(OUT_FILE): $(SRC_FILE)
	$(CC) $(CFLAGS) -g -o $(OUT_FILE) $(SRC_FILE)

$(ASM_FILE): $(SRC_FILE)
	$(CC) $(CFLAGS) -S -o $(ASM_FILE) $(SRC_FILE)

clean:
	rm $(OUTFILES)

test:
	./$(OUT_FILE) test_file.wav
