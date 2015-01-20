include ./compile.mk

SOURCE_DIR = ../src
CFLAGS = -Wall -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lmosquitto -ljansson

EXEC = sanji_controller
SRCS = sanji_controller.c resource.c component.c session.c
OBJS = $(SRCS:.c=.o)

#VERBOSE = yes
ifdef VERBOSE
CFLAGS += -DVERBOSE
EXEC = sanji_controller_verbose
endif

DEBUG = yes
ifdef DEBUG
CFLAGS += -DDEBUG
EXEC = sanji_controller_debug
endif


.PHONY: all clean

all: $(EXEC)
	-rm *.o

$(EXEC): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	$(STRIP) -s $@

%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $<

clean: 
	-rm -f *.o sanji_controller sanji_controller_debug sanji_controller_verbose
