include ./compile.mk

SOURCE_DIR = ../src
CFLAGS = -Wall -I/usr/local/include -I$(SOURCE_DIR)/lib
LDFLAGS = -L/usr/local/lib -lmosquitto -ljansson

EXEC = sanji_controller
SRCS = sanji_controller.c resource.c component.c session.c ini.c
OBJS = $(SRCS:.c=.o)
LIBS = libsanjimisc.a

#VERBOSE = yes
ifdef VERBOSE
CFLAGS += -DVERBOSE
EXEC = sanji_controller_verbose
endif

#DEBUG = yes
ifdef DEBUG
CFLAGS += -DDEBUG
EXEC = sanji_controller_debug
endif


.PHONY: all clean

all: $(LIBS) $(EXEC)
	-rm *.o

$(LIBS):
	make -C $(SOURCE_DIR)/lib/build
	cp $(SOURCE_DIR)/lib/build/$(LIBS) .

$(EXEC): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	$(STRIP) -s $@

%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $<

clean: 
	make -C $(SOURCE_DIR)/lib/build clean
	-rm -f *.o $(LIBS) sanji_controller sanji_controller_debug sanji_controller_verbose
