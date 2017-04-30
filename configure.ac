#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT( [difflink], [1.0.0], [snyder.bradleyj@gmail.com])
AM_INIT_AUTOMAKE(difflink, 1.0.0)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

PKG_CHECK_MODULES([DEPS], [glib-2.0 >= 2.24.1])

# Search for libraries
AC_SEARCH_LIBS( [g_hash_table_new], [glib-2.0], [], [] )

# Checks for libraries.
AC_CHECK_LIB( [glib-2.0],   [g_hash_table_new], [], [AC_MSG_ERROR( [libglib-2.0 not found.] )] )

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/param.h unistd.h assert.h glib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
