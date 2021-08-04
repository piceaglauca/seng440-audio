CC=armv7hl-redhat-linux-gnueabi-gcc
CFLAGS=-Wall

LOOKUP_TABLE_SRC=main_lookuptable.c
LOOKUP_TABLE_ASM=main_lookuptable.s
LOOKUP_TABLE_OUT=main_lookuptable

OPT_SRC=main_optimized.c
OPT_ASM=main_optimized.s
OPT_OUT=main_optimized

ORIGINAL_SRC=main_original.c
ORIGINAL_ASM=main_original.s
ORIGINAL_OUT=main_original

OUTFILES=$(ORIGINAL_OUT) $(ORIGINAL_ASM) $(LOOKUP_TABLE_OUT) $(LOOKUP_TABLE_ASM) $(OPT_OUT) $(OPT_ASM)

.PHONY: opt lookup_table original clean test_lookup_table test_opt test_original test all

all: opt lookup_table original

original: $(ORIGINAL_OUT) $(ORIGINAL_ASM)

$(ORIGINAL_OUT): $(ORIGINAL_SRC)
	$(CC) $(CFLAGS) -g -o $(ORIGINAL_OUT) $(ORIGINAL_SRC)

$(ORIGINAL_ASM): $(ORIGINAL_SRC)
	$(CC) $(CFLAGS) -S -o $(ORIGINAL_ASM) $(ORIGINAL_SRC)


lookup_table: $(LOOKUP_TABLE_OUT) $(LOOKUP_TABLE_ASM)

$(LOOKUP_TABLE_OUT): $(LOOKUP_TABLE_SRC)
	$(CC) $(CFLAGS) -g -o $(LOOKUP_TABLE_OUT) $(LOOKUP_TABLE_SRC)

$(LOOKUP_TABLE_ASM): $(LOOKUP_TABLE_SRC)
	$(CC) $(CFLAGS) -S -o $(LOOKUP_TABLE_ASM) $(LOOKUP_TABLE_SRC)


opt: $(OPT_OUT) $(OPT_ASM)

$(OPT_ASM): $(OPT_SRC)
	$(CC) $(CFLAGS) -S -o $(OPT_ASM) $(OPT_SRC)

$(OPT_OUT): $(OPT_SRC)
	$(CC) $(CFLAGS) -g -o $(OPT_OUT) $(OPT_SRC)


clean:
	rm $(OUT_FILES)

test_original:
	./$(ORIGINAL_OUT) test_file.wav

test_lookup_table:
	./$(LOOKUP_TABLE_OUT) test_file.wav

test_opt:
	./$(OPT_OUT) test_file.wav

test: test_opt
