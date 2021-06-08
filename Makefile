CC=gcc
CFLAGS=-g -Wall

all: unopt_main

unopt_main: unopt_main.c
	$(CC) $(CFLAGS) -o unopt_main unopt_main.c

clean:
	rm unopt_main

test:
	./unopt_main test_file.wav
