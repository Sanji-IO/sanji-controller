SUBDIRS = lib

libdir = lib
bin_PROGRAMS = sanji-controller
sanji_controller_SOURCES = \
	debug.h \
	error.h \
	http.h \
	list.h \
	sanji_controller.h sanji_controller.c \
	resource.h resource.c \
	component.h component.c \
	session.h session.c \
	ini.h ini.c
sanji_controller_LDFLAGS = $(mosquitto_LIBS) $(jansson_LIBS)
sanji_controller_LDADD = $(libdir)/libsanjimisc.a

AM_CPPFLAGS = -I$(libdir)
