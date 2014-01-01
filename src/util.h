/* Hydra (c) 2013 Oleg Efimov */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_ERROR 1
#define LOG_INFO  2
#define LOG_DEBUG 3

void hlog(unsigned int log_level, char* log_message)
{
  char log_date_and_time[20];
  time_t t = time(0);
  struct tm *timeptr;

  timeptr = gmtime(&t);
  strftime(log_date_and_time, sizeof(log_date_and_time), "%Y-%m-%d %H:%M:%S", timeptr);
  log_date_and_time[sizeof(log_date_and_time) - 1] = '\0'; //TODO: Are we need this?
  printf("[%s] ", log_date_and_time);

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

  printf("%s\n", log_message);
}

void inline hlog_error(char* log_message)
{
  hlog(LOG_ERROR, log_message);
}

void inline hlog_info(char* log_message)
{
  hlog(LOG_INFO, log_message);
}

void inline hlog_debug(char* log_message)
{
  hlog(LOG_DEBUG, log_message);
}
