SOURCE_DIR = ..

include ./compile.mk

CFLAGS = -Wall -g
ifdef DEBUG
CFLAGS += -DDEBUG
endif

SRCS = \
       crc16.c \
       daemonize.c \
       dt.c \
       time_util.c \
       lock.c \
	   pid.c \
       text_util.c

OBJS = $(SRCS:.c=.o)

TARGET = libsanjimisc.a

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o $(TARGET) *.gdb

