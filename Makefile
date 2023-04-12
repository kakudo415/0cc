CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

0cc: $(OBJS)
	$(CC) -o 0cc $(OBJS) $(LDFLAGS)

$(OBJS): 0cc.h

test: 0cc
	./test.sh

clean:
	rm -f 0cc *.o *~ tmp*

.PHONY: test clean

