/*
 * Copyright (c) 2013-2016 Oleg Efimov <efimovov@gmail.com>
 *
 * hydrad is free software; you can redistribute it
 * and/or modify it under the terms of the MIT license.
 * See LICENSE for details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_ERROR 1
#define LOG_INFO  2
#define LOG_DEBUG 3

void hlog(unsigned int log_level, char* log_message, ...)
{
  va_list arguments;
  va_start(arguments, log_message);

  char log_date_and_time[20];
  time_t t = time(0);
  struct tm *timeptr;

  timeptr = gmtime(&t);
  strftime(log_date_and_time, sizeof(log_date_and_time), "%Y-%m-%d %H:%M:%S", timeptr);
  log_date_and_time[sizeof(log_date_and_time) - 1] = '\0'; // TODO: Are we need this?
  printf("[%s] ", log_date_and_time);

  printf("[pid=%d] ", getpid());

  switch (log_level) {
    case LOG_ERROR:
      printf("[ERROR] ");
      break;
    case LOG_INFO:
      printf("[INFO] ");
      break;
    case LOG_DEBUG:
      printf("[DEBUG] ");
      break;
    default:
      printf("[UNKNOWN] ");
      break;
  }

  vprintf(log_message, arguments);
  printf("\n");

  va_end(arguments);
}

#define hlog_error(log_message, ...) hlog(LOG_ERROR, log_message, ##__VA_ARGS__)
#define hlog_info(log_message, ...)  hlog(LOG_INFO, log_message, ##__VA_ARGS__)
#define hlog_debug(log_message, ...) hlog(LOG_DEBUG, log_message, ##__VA_ARGS__)

// TODO: daemonize
// http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
// http://codingfreak.blogspot.com/2012/03/daemon-izing-process-in-linux.html
