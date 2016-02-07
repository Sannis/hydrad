dnl Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
dnl
dnl hydrad is free software; you can redistribute it
dnl and/or modify it under the terms of the MIT license.
dnl See LICENSE for details.

AC_PREREQ([2.59])
AC_INIT([hydrad], [0.0.1-dev], [efimovov@gmail.com])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign -Wall])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_ENABLE_SHARED
AC_ENABLE_STATIC

AC_PROG_CC
AC_PROG_MAKE_SET

AM_SILENT_RULES([yes])

LT_INIT

AC_MSG_RESULT([])

AX_PROTOBUF_C
AX_LIBJANSSON
AX_PROTOBUF2JSON_C
AX_LIBUV

AC_MSG_RESULT([])

MY_SANITIZE
MY_VALGRIND
MY_COVERAGE

AC_MSG_RESULT([])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
