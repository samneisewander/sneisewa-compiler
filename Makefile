.PHONY: test
CC=gcc
CFLAGS=-Wall -g -std=gnu99
LD=gcc

bminor: bminor.o scanner.o
	$(CC) $(CFLAGS) -o $@ $^

bminor.o: bminor.c
	$(CC) $(CFLAGS) -o $@ -c $^

scanner.o: scanner.c
	$(CC) $(CFLAGS) -o $@ -c $^

scanner.c: scanner.flex
	flex -o $@ $^

test:
	@echo "Running tests..."
	chmod +x runtest.sh
	./runtest.sh

clean:
	@echo "Cleaning up..."
	rm -f *.o bminor scanner.c
	rm -f test/*/*.out

