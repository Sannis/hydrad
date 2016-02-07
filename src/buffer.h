/*
 * Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
 *
 * hydrad is free software; you can redistribute it
 * and/or modify it under the terms of the MIT license.
 * See LICENSE for details.
 *
 * Based on buffer.[ch] from https://github.com/clibs/buffer
 * Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>

/*
 * Default buffer size.
 */

#ifndef BUFFER_DEFAULT_SIZE
#define BUFFER_DEFAULT_SIZE 64
#endif

/*
 * Buffer struct.
 */

typedef struct {
  size_t len;
  char *alloc;
  char *data;
} buffer_t;

/*
 * Compute the nearest multiple of `a` from `b`.
 */

#define nearest_multiple_of(a, b) \
  (((b) + ((a) - 1)) & ~((a) - 1))

/*
 * Allocate a new buffer with `n` bytes.
 */

buffer_t *
buffer_new_with_size(size_t n) {
  buffer_t *self = malloc(sizeof(buffer_t));
  if (!self) return NULL;
  self->len = n;
  self->data = self->alloc = calloc(n + 1, 1);
  return self;
}

/*
 * Allocate a new buffer with BUFFER_DEFAULT_SIZE.
 */

buffer_t *
buffer_new() {
  return buffer_new_with_size(BUFFER_DEFAULT_SIZE);
}

/*
 * Allocate a new buffer with `str` and `len`.
 */

buffer_t *
buffer_new_with_string_length(char *str, size_t len) {
  buffer_t *self = malloc(sizeof(buffer_t));
  if (!self) return NULL;
  self->len = len;
  self->data = self->alloc = str;
  return self;
}

/*
 * Allocate a new buffer with `str`.
 */

buffer_t *
buffer_new_with_string(char *str) {
  return buffer_new_with_string_length(str, strlen(str));
}

/*
 * Allocate a new buffer with a copy of `str`.
 */

buffer_t *
buffer_new_with_copy(char *str) {
  size_t len = strlen(str);
  buffer_t *self = buffer_new_with_size(len);
  if (!self) return NULL;
  memcpy(self->alloc, str, len);
  self->data = self->alloc;
  return self;
}

/*
 * Free the buffer.
 */

void
buffer_free(buffer_t *self) {
  free(self->alloc);
  free(self);
}

/*
 * Return buffer size.
 */

size_t
buffer_size(buffer_t *self) {
  return self->len;
}

/*
 * Return string length.
 */

size_t
buffer_length(buffer_t *self) {
  return strlen(self->data);
}

/*
 * Return string content.
 */

char *
buffer_string(buffer_t *self) {
  return self->data;
}

/*
 * Resize to hold `n` bytes.
 */

int
buffer_resize(buffer_t *self, size_t n) {
  n = nearest_multiple_of(1024, n);
  self->len = n;
  self->alloc = self->data = realloc(self->alloc, n + 1);
  if (!self->alloc) return -1;
  self->alloc[n] = '\0';
  return 0;
}

/*
 * Append the first `len` bytes from `str` to `self` and
 * return 0 on success, -1 on failure.
 */
int
buffer_append_n(buffer_t *self, const char *str, size_t len) {
  size_t prev = strlen(self->data);
  size_t needed = len + prev;

  // enough space
  if (self->len > needed) {
    strncat(self->data, str, len);
    return 0;
  }

  // resize
  int ret = buffer_resize(self, needed);
  if (-1 == ret) return -1;
  strncat(self->data, str, len);

  return 0;
}

/*
 * Append `str` to `self` and return 0 on success, -1 on failure.
 */

int
buffer_append(buffer_t *self, const char *str) {
  return buffer_append_n(self, str, strlen(str));
}

/*
 * Return a new buffer based on the `from..to` slice of `buf`,
 * or NULL on error.
 */

buffer_t *
buffer_slice(buffer_t *buf, size_t from, ssize_t to) {
  size_t len = strlen(buf->data);

  // bad range
  if (to < from) return NULL;

  // relative to end
  if (to < 0) to = len - ~to;

  // cap end
  if (to > len) to = len;

  size_t n = to - from;
  buffer_t *self = buffer_new_with_size(n);
  memcpy(self->data, buf->data + from, n);
  return self;
}

/*
 * Return the index of the substring `str`, or -1 on failure.
 */

ssize_t
buffer_indexof(buffer_t *self, char *str) {
  char *sub = strstr(self->data, str);
  if (!sub) return -1;
  return sub - self->data;
}