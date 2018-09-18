#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIRS([m4])
AC_INIT([The Practical Programming Language], [0.0], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([examples/hello.pr])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT

AC_LANG(C++)
# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_MKDIR_P

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([cppunit/extensions/HelperMacros.h], [], [AC_MSG_FAILURE([CppUnit headers not found])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 lib/Makefile])
AC_OUTPUT
