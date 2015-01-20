include ../build/compile.mk

SOURCE_DIR = ..
CFLAGS = -Wall -I/usr/local/include -I..
LDFLAGS = -L/usr/local/lib -ljansson -lmosquitto

#EXEC = component_unit_test
EXEC = resource_unit_test
#EXEC = session_unit_test
SRCS = $(EXEC).c resource.c component.c session.c
OBJS = $(SRCS:.c=.o)

DEBUG = yes
ifdef DEBUG
CFLAGS += -DDEBUG
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
	-rm -f *.o $(EXEC) component_example resource_example session_example component_unit_test resource_unit_test session_unit_test
