CC=armv7hl-redhat-linux-gnueabi-gcc
CFLAGS=-Wall

SRC_FILE=main.c
ASM_FILE=main.s
OUT_FILE=main

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
