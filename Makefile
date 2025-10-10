.PHONY: test
CC=gcc
CFLAGS=-Wall -g -std=gnu99
LD=gcc
PARSEGEN=bison
SCANGEN=flex

bminor: bminor.o scanner.o parser.o
	$(CC) $(CFLAGS) -o $@ $^

bminor.o: bminor.c token.h
	$(CC) $(CFLAGS) -o $@ -c $<

scanner.o: scanner.c
	$(CC) $(CFLAGS) -o $@ -c $^

parser.o: parser.c
	$(CC) $(CFLAGS) -o $@ -c $^

scanner.c: scanner.flex token.h
	$(SCANGEN) -o $@ $^

token.h parser.c: parser.bison
	$(PARSEGEN) -v --defines=token.h --output=parser.c $^

test:
	@echo "Running tests..."
	chmod +x runtest.sh
	./runtest.sh

clean:
	@echo "Cleaning up..."
	rm -f *.o bminor scanner.c parser.c token.h parser.output
	rm -f test/*/*.out

