#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sanji-controller], [1.0], [aeluin.chen@moxa.com])
AM_INIT_AUTOMAKE([1.11 foreign])
AC_CONFIG_SRCDIR([src/sanji_controller.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_SUBST(mosquitto_LIBS)
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([jansson], [json_object])
AC_CHECK_LIB([mosquitto], [mosquitto_lib_init],
	[mosquitto_LIBS="-lmosquitto"],
	AC_ERROR([Misssing mosquitto client library]))

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime gethostname memmove memset strchr strerror strrchr utime])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/lib/Makefile])
AC_OUTPUT
