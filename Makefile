.PHONY: test
CC=gcc
CFLAGS=-Wall -g -std=gnu99
LD=gcc
LIBS=

bminor: bminor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	$(MAKE) test

test:
	@echo "Running tests..."
	chmod +x runtest.sh
	./runtest.sh	

clean:
	@echo "Cleaning up..."
	rm -f *.o bminor
	rm -f test/encode/*.out

