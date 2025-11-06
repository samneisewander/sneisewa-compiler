# Configuration
.PHONY: test
CC=gcc
CFLAGS=-Wall -g -std=gnu99 -Iinclude
LD=gcc
LDFLAGS=
LIBS=

# bminor depends on everything in the source directory, plus the scanner and parser C output files from flex/bison
BMINOR_SOURCES= $(wildcard src/*.c)
BMINOR_OBJECTS=	$(BMINOR_SOURCES:src/%.c=build/%.o) build/parser.o build/scanner.o

COMPILER=bin/bminor

all: $(COMPILER)

$(COMPILER): $(BMINOR_OBJECTS) token.h
	@echo "Linking $@"
	@$(LD) $(LDFLAGS) -o $@ $(BMINOR_OBJECTS) $(LIBS)

# Rules
parser.o: build/parser.c
	@$(CC) $(CFLAGS) -c -o build/$@ $<

scanner.o: build/scanner.c
	@$(CC) $(CFLAGS) -c -o build/$@ $<

build/%.o: src/%.c token.h
	@echo "Compiling object dependency $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

build/scanner.c: src/scanner.flex token.h
	@echo "Generating scanner source $@"
	flex -o $@ $<

token.h build/parser.c: src/parser.bison
	@echo "Generating parser source and token header file"
	bison --defines=include/token.h --output=build/parser.c $^

test: all
	@echo "Running tests..."
	chmod +x runtest.sh
	./runtest.sh

clean:
	@echo "Cleaning up..."
	rm -f build/*
	rm -f bin/*
	rm -f include/token.h
	rm -f test/*/*.out test/*/*.pretty

