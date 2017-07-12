#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "logger.h"

static const char* levels[] = {"ERROR","WARNING","INFO","DEBUG"};

void logging(int level, const char* message, ...) {
  if(LOG_LEVEL>=level) {
    va_list    args;
    va_start(args, message);
    time_t now;
    time(&now);
    if(0==level) {
      char* p = ctime(&now);
      fprintf(stderr, "%.*s %s:\t",(int)strlen(p)-1, p,levels[level]);
      vfprintf(stderr,message,args);
      fputc('\n', stderr);
    }
    else {
      char* p = ctime(&now);
      printf("%.*s %s:\t",(int)strlen(p)-1, p,levels[level]);
      vprintf(message,args);
      printf("\n");
    }
    va_end(args);
  }
}
