CC=gcc
CFLAGS=-g -Wall

UNOPT_SRC=unopt_main.c
UNOPT_OUT=unopt_main
OPT_SRC=opt_main.c
OPT_OUT=opt_main.asm
OUTFILES=$(UNOPT_OUT) $(OPT_OUT)

.PHONY: unopt
unopt: unopt_main

$(UNOPT_OUT): $(UNOPT_SRC)
	$(CC) $(CFLAGS) -o $(UNOPT_OUT) $(UNOPT_SRC)

.PHONY: opt
opt: $(OPT_OUT)

$(OPT_OUT): $(OPT_SRC)
	$(CC) $(CFLAGS) -S -o $(OPT_OUT) $(OPT_SRC)

clean:
	rm $(OUT_FILES)

test:
	./$(UNOPT_OUT) test_file.wav

all: 
	unopt
	opt
