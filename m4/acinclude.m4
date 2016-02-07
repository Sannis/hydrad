dnl Copyright (c) 2014-2016 Oleg Efimov <efimovov@gmail.com>
dnl
dnl protobuf2json-c is free software; you can redistribute it
dnl and/or modify it under the terms of the MIT license.
dnl See LICENSE for details.

AC_DEFUN([AX_PROTOBUF_C],
[
  AC_ARG_WITH([protobuf_c], AC_HELP_STRING([--with-protobuf-c=DIR], [protobuf-c install directory]))

  PROTOBUF_C_INCLUDES=""
  PROTOBUF_C_CFLAGS=""
  PROTOBUF_C_LIBS=""
  PROTOBUF_C_COMPILER=""

  have_protobuf_c=false
  if test "$1" != "optional" -o \( "$with_protobuf_c" != "no" -a -n "$with_protobuf_c" \) ; then
    AC_MSG_CHECKING([for protobuf-c headers])

    protobuf_c_path=""
    for path in "$with_protobuf_c" /usr/local /usr /opt/local /opt ; do
      if test -r "$path/include/google/protobuf-c/protobuf-c.h" -a "$path/bin/protoc-c" ; then
        protobuf_c_path=$path
        break
      fi
    done

    if test -n "$protobuf_c_path" ; then
      have_protobuf_c=true
      AC_MSG_RESULT([$protobuf_c_path])
      PROTOBUF_C_INCLUDES="-I$protobuf_c_path/include"
      PROTOBUF_C_LIBS="-L$protobuf_c_path/lib -lprotobuf-c"
      PROTOBUF_C_CFLAGS="-DPRINT_UNPACK_ERRORS=0"
      PROTOBUF_C_COMPILER="$path/bin/protoc-c"
    else
      AC_MSG_ERROR([protobuf_c headers not found])
    fi
  fi

  AM_CONDITIONAL([ENABLE_PROTOBUF_C], [test "x$have_protobuf_c" = "xtrue"])

  if test "x$have_protobuf_c" = "xtrue" ; then
    AC_MSG_RESULT([protobuf_c: INCLUDES=$PROTOBUF_C_INCLUDES, LIBS=$PROTOBUF_C_LIBS, CFLAGS=$PROTOBUF_C_CFLAGS, PROTOC-C=$PROTOBUF_C_COMPILER])
  fi

  AC_SUBST([PROTOBUF_C_INCLUDES])
  AC_SUBST([PROTOBUF_C_LIBS])
  AC_SUBST([PROTOBUF_C_CFLAGS])
  AC_SUBST([PROTOBUF_C_COMPILER])
])

AC_DEFUN([AX_LIBJANSSON],
[
  AC_ARG_WITH([libjansson], AC_HELP_STRING([--with-libjansson=DIR], [libjansson install directory]))

  LIBJANSSON_INCLUDES=""
  LIBJANSSON_LIBS=""

  have_libjansson=false
  if test "$1" != "optional" -o \( "$with_libjansson" != "no" -a -n "$with_libjansson" \) ; then
    AC_MSG_CHECKING([for libjansson headers])

    libjansson_path=""
    for dir in "$with_libjansson" /usr/local /usr /opt/local /opt ; do
      if test -r "$dir/include/jansson.h" ; then
        libjansson_path="$dir"
        break
      fi
    done

    if test -n "$libjansson_path" ; then
      have_libjansson=true
      AC_MSG_RESULT([$libjansson_path])
      LIBJANSSON_INCLUDES="-I$libjansson_path/include"
      LIBJANSSON_LIBS="-L$libjansson_path/lib -ljansson"
    else
      AC_MSG_ERROR([libjansson headers not found])
    fi
  fi

  AM_CONDITIONAL([ENABLE_LIBJANSSON], [test "x$have_libjansson" = "xtrue"])

  if test "x$have_libjansson" = "xtrue" ; then
    AC_MSG_RESULT([libjansson: INCLUDES=$LIBJANSSON_INCLUDES, LIBS=$LIBJANSSON_LIBS])
  fi

  AC_SUBST([LIBJANSSON_INCLUDES])
  AC_SUBST([LIBJANSSON_LIBS])
])

AC_DEFUN([AX_PROTOBUF2JSON_C],
[
  AC_ARG_WITH([protobuf2json_c], AC_HELP_STRING([--with-protobuf2json-c=DIR], [protobuf2json-c install directory]))

  PROTOBUF2JSON_C_INCLUDES=""
  PROTOBUF2JSON_C_LIBS=""

  have_protobuf2json_c=false
  if test "$1" != "optional" -o \( "$with_protobuf2json_c" != "no" -a -n "$with_protobuf2json_c" \) ; then
    AC_MSG_CHECKING([for protobuf2json-c headers])

    protobuf2json_c_path=""
    for dir in "$with_protobuf2json_c" /usr/local /usr /opt/local /opt ; do
      if test -r "$dir/include/jansson.h" ; then
        protobuf2json_c_path="$dir"
        break
      fi
    done

    if test -n "$protobuf2json_c_path" ; then
      have_protobuf2json_c=true
      AC_MSG_RESULT([$protobuf2json_c_path])
      PROTOBUF2JSON_C_INCLUDES="-I$protobuf2json_c_path/include"
      PROTOBUF2JSON_C_LIBS="-L$protobuf2json_c_path/lib -lprotobuf2json-c"
    else
      AC_MSG_ERROR([protobuf2json-c headers not found])
    fi
  fi

  AM_CONDITIONAL([ENABLE_PROTOBUF2JSON_C], [test "x$have_protobuf2json_c" = "xtrue"])

  if test "x$have_protobuf2json_c" = "xtrue" ; then
    AC_MSG_RESULT([protobuf2json-c: INCLUDES=$PROTOBUF2JSON_C_INCLUDES, LIBS=$PROTOBUF2JSON_C_LIBS])
  fi

  AC_SUBST([PROTOBUF2JSON_C_INCLUDES])
  AC_SUBST([PROTOBUF2JSON_C_LIBS])
])

AC_DEFUN([AX_LIBUV],
[
  AC_ARG_WITH([libuv], AC_HELP_STRING([--with-libuv=DIR], [libuv install directory]))

  LIBUV_INCLUDES=""
  LIBUV_LIBS=""

  have_libuv=false
  if test "$1" != "optional" -o \( "$with_libuv" != "no" -a -n "$with_libuv" \) ; then
    AC_MSG_CHECKING([for libuv headers])

    libuv_path=""
    for dir in "$with_libuv" /usr/local /usr /opt/local /opt ; do
      if test -r "$dir/include/uv.h" ; then
        libuv_path="$dir"
        break
      fi
    done

    if test -n "$libuv_path" ; then
      have_libuv=true
      AC_MSG_RESULT([$libuv_path])
      LIBUV_INCLUDES="-I$libuv_path/include"
      LIBUV_LIBS="-L$libuv_path/lib -luv"
    else
      AC_MSG_ERROR([libuv headers not found])
    fi
  fi

  AM_CONDITIONAL([ENABLE_LIBUV], [test "x$have_libuv" = "xtrue"])

  if test "x$have_libuv" = "xtrue" ; then
    AC_MSG_RESULT([libuv: INCLUDES=$LIBUV_INCLUDES, LIBS=$LIBUV_LIBS])
  fi

  AC_SUBST([LIBUV_INCLUDES])
  AC_SUBST([LIBUV_LIBS])
])