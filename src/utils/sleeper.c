#define _XOPEN_SOURCE 500
#include "sleeper.h"

#include <time.h>

int msleep(const long t) {
  struct timespec tv = {.tv_nsec = t * 1000 * 1000};
  return nanosleep(&tv, NULL);
}
