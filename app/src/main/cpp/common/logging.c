//
// Created by kirill on 15.05.20.
//

#include <string.h>
#include <errno.h>
#include <time.h>
#include "logging.h"

static FILE* g_logfile = NULL;

int init_log_file(const char* path)
{
  if (g_logfile != NULL)
  {
    fclose(g_logfile);
  }

  g_logfile = fopen(path, "w+");
  if (g_logfile == NULL)
  {
    LOGV("Unable to open log file at \"%s\": %s!", path, strerror(errno));
    return -1;
  }

  return 0;
}

void log_to_file(const char* format, ...)
{
  if (g_logfile != NULL)
  {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(g_logfile, "%02d-%02d %02d:%02d:%02d ", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
            tm.tm_min, tm.tm_sec);
    va_list args;
    va_start(args, format);
    va_end(args);
    vfprintf(g_logfile, format, args);
    fprintf(g_logfile, "\n");
    fflush(g_logfile);
  }
}
