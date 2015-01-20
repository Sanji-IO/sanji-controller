COMPILER = gcc
CROSS_PREFIX = 
CC = $(CROSS_PREFIX)$(COMPILER)
LD = $(CROSS_PREFIX)$(COMPILER)
RM = rm -f
CFLAGS = -Wall -I..
LDFLAGS = -L../build -lsanjimisc -lrt

SRCS = \
	bswap_example.c \
	crc16_example.c \
	daemonize_example.c \
	dt_example.c \
	time_util_example.c \
	lock_example.c \
	pid_example.c \
	text_util_example.c \
	strext_example.c \
	random_util_example.c

OBJS = $(SRCS:.c=.o)
PROGS = $(SRCS:.c=)

.PHONY: all clean

all: $(word 1,$(PROGS))

$(word 1,$(PROGS)): $(OBJS)
	for prog in $(PROGS); \
	do \
		$(LD) -o $${prog} $${prog}.o $(LDFLAGS); \
	done

.SUFFIXES: .c .o
.c.o: 
	$(CC) $(CFLAGS) -c $<		   

clean:
	@-$(RM) $(PROGS) $(OBJS)
