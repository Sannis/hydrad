# Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
#
# hydrad is free software; you can redistribute it
# and/or modify it under the terms of the MIT license.
# See LICENSE for details.

bin_PROGRAMS = hydrad

hydrad_SOURCES = hydrad.c \
                 hydrad.pb-c.c

AM_CFLAGS = -I$(top_srcdir)/include
hydrad_LDADD =

AM_CFLAGS += $(PROTOBUF_C_INCLUDES)
hydrad_LDADD += $(PROTOBUF_C_LIBS)
AM_CFLAGS += $(PROTOBUF_C_CFLAGS)

AM_CFLAGS += $(LIBJANSSON_INCLUDES)
hydrad_LDADD += $(LIBJANSSON_LIBS)

AM_CFLAGS += $(PROTOBUF2JSON_C_INCLUDES)
hydrad_LDADD += $(PROTOBUF2JSON_C_LIBS)

AM_CFLAGS += $(LIBUV_INCLUDES)
hydrad_LDADD += $(LIBUV_LIBS)

AM_CFLAGS += $(MY_SANITIZE_CFLAGS)
hydrad_LDADD += $(MY_SANITIZE_LIBS)

AM_CFLAGS += $(MY_VALGRIND_CFLAGS)
hydrad_LDADD += $(MY_VALGRIND_LDFLAGS)

AM_CFLAGS += $(MY_COVERAGE_CFLAGS)
hydrad_LDADD += $(MY_COVERAGE_LDFLAGS)

BUILT_SOURCES = hydrad.pb-c.c

PROTO_DIR = $(top_srcdir)/proto

%.pb-c.c %.pb-c.h: $(PROTO_DIR)/%.proto
	$(PROTOBUF_C_COMPILER) -I$(PROTO_DIR) --c_out=. $<

MOSTLYCLEANFILES =
MOSTLYCLEANFILES += *.pb-c.[ch]
