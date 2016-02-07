#!/bin/sh

# Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
#
# hydrad is free software; you can redistribute it
# and/or modify it under the terms of the MIT license.
# See LICENSE for details.

cd `dirname "$0"`

if [ "$LIBTOOLIZE" = "" ] && [ "`uname`" = "Darwin" ]; then
  LIBTOOLIZE=glibtoolize
fi

ACLOCAL=${ACLOCAL:-aclocal}
AUTOCONF=${AUTOCONF:-autoconf}
AUTOMAKE=${AUTOMAKE:-automake}
LIBTOOLIZE=${LIBTOOLIZE:-libtoolize}

set -ex
"$LIBTOOLIZE"
"$ACLOCAL" -I m4
"$AUTOCONF"
"$AUTOMAKE" --add-missing --copy
