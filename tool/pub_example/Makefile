CFLAGS = -Wall -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lmosquitto

.PHONY: all clean

all: pub_example

sub_example: pub_example.o
	${CC} $^ -o $@ ${LDFLAGS}

sub_example.o: pub_example.c
	${CC} -c $^ -o $@ ${CFLAGS}

clean: 
	-rm -f *.o pub_example
