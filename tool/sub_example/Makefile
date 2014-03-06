CFLAGS = -Wall -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lmosquitto

.PHONY: all clean

all: sub_example sub_example_simple

sub_example: sub_example.o
	${CC} $^ -o $@ ${LDFLAGS}

sub_example.o: sub_example.c
	${CC} -c $^ -o $@ ${CFLAGS}

sub_example_simple: sub_example_simple.o
	${CC} $^ -o $@ ${LDFLAGS}

sub_example_simple.o: sub_example_simple.c
	${CC} -c $^ -o $@ ${CFLAGS}


clean: 
	-rm -f *.o sub_example sub_example_simple
