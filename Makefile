#
# Makefile for 'pgplay'.
#
# Type 'make' or 'make pgplay' to create the binary.
# Type 'make clean' or 'make clear' to delete all temporaries.
# Type 'make run' to execute the binary.
# Type 'make debug' to debug the binary using gdb(1).
#

# build target specs
CC = gcc
CFLAGS = -O3 
OUT_DIR = release_build
LIBS = -lpq

# first target entry is the target invoked when typing 'make'
default: pgplay

pgplay: $(OUT_DIR)/pgplay.c.o
	@echo -n 'Linking pgplay... '
	@$(CC) $(CFLAGS) -o pgplay $(OUT_DIR)/pgplay.c.o $(LIBS)
	@echo Done.

$(OUT_DIR)/pgplay.c.o: pgplay.c
	@echo -n 'Compiling pgplay.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/pgplay.c.o -c pgplay.c
	@echo Done.

run:
	./pgplay 

debug:
	gdb ./pgplay

clean:
	@echo -n 'Removing all temporary binaries... '
	@rm -f pgplay $(OUT_DIR)/*.o
	@echo Done.

clear:
	@echo -n 'Removing all temporary binaries... '
	@rm -f pgplay $(OUT_DIR)/*.o
	@echo Done.

