CC=gcc
CFLAGS=-g -Wall

UNOPT_SRC=unopt_main.c
UNOPT_OUT=unopt_main
OPT_SRC=opt_main.c
OPT_ASM=opt_main.asm
OPT_OUT=opt_main
OUTFILES=$(UNOPT_OUT) $(OPT_OUT) $(OPT_ASM)

.PHONY: unopt
unopt: unopt_main

$(UNOPT_OUT): $(UNOPT_SRC)
	$(CC) $(CFLAGS) -o $(UNOPT_OUT) $(UNOPT_SRC)

.PHONY: opt
opt: $(OPT_OUT)

$(OPT_OUT): $(OPT_SRC)
	$(CC) $(CFLAGS) -S -o $(OPT_ASM) $(OPT_SRC)
	$(CC) $(CFLAGS) -o $(OPT_OUT) $(OPT_SRC)

clean:
	rm $(OUT_FILES)

.PHONY: testunopt
testunopt:
	./$(UNOPT_OUT) test_file.wav

.PHONY: testopt
testopt:
	./$(OPT_OUT) test_file.wav

test: testopt

all: 
	unopt
	opt
