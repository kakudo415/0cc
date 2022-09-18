CFLAGS=-std=c11 -g -static

0cc: 0cc.c

test: 0cc
	./test.sh

clean:
	rm -f 0cc *.o *~ tmp*

.PHONY: test clean

