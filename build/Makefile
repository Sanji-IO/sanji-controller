include ./compile.mk

SOURCE_DIR = ..
CFLAGS = -Wall -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lmosquitto -ljansson

EXEC = sanji_controller
SRCS = sanji_controller.c resource.c component.c session.c
OBJS = $(SRCS:.c=.o)

VERBOSE = yes
ifdef VERBOSE
CFLAGS += -DVERBOSE
endif

#DEBUG = yes
ifdef DEBUG
CFLAGS += -DDEBUG
endif


.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	$(STRIP) -s $@

%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $<

clean: 
	-rm -f *.o sanji_controller
