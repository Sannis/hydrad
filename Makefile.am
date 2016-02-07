# Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
#
# hydrad is free software; you can redistribute it
# and/or modify it under the terms of the MIT license.
# See LICENSE for details.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

cppcheck:
	cppcheck --enable=style src/hydrad.c src/buffer.h src/util.h --error-exitcode=2

test: check
	@MY_VALGRIND_EXEC_PREFIX@python tests/all.py

MY_COVERAGE_DIRECTORY = $(abs_top_builddir)/src
@MY_COVERAGE_RULES@
