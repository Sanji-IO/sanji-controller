ARCH = x86

ifeq ($(ARCH),x86)
CROSS =
CFLAGS =
LDGLAGS =
endif

ifeq ($(ARCH),moxaart-2.6.38)
CROSS = arm-linux-
CFLAGS += -I/usr/local/arm-linux-4.4.2/include
LDFLAGS += -L/usr/local/arm-linux-4.4.2/lib
endif

ifeq ($(ARCH),moxaart-2.6.9)
CROSS = arm-linux-
CFLAGS += -I/usr/local/arm-linux-1.3.1/include
LDFLAGS += -L/usr/local/arm-linux-1.3.1/lib
endif

ifeq ($(ARCH),xscale)
CROSS = xscale-none-linux-gnueabi-
CFLAGS += -DWORDS_BIGENDIAN -DUC848X -I/usr/local/arm-linux-4.4.2-v4/xscale-none-linux-gnueabi/include
LDFLAGS += -L/usr/local/arm-linux-4.4.2-v4/xscale-none-linux-gnueabi/lib
endif

ifeq ($(ARCH),marvell)
CROSS = arm-mv5sft-linux-gnueabi-
CFLAGS = -I/usr/local/arm-mv5sft-linux-gnueabi/include
LDGLAGS = -L/usr/local/arm-mv5sft-linux-gnueabi/lib
endif


CC = $(CROSS)gcc
LD = $(CROSS)gcc
STRIP = $(CROSS)strip
AR = $(CROSS)ar
CFLAGS += -Wall -fPIC
LDFLAGS +=
