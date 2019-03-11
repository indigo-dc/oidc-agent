#define _XOPEN_SOURCE 500
#include "sleeper.h"

#include <syslog.h>
#include <time.h>

int msleep(const long t) {
  const unsigned long nano    = t * 1000 * 1000;
  const unsigned long seconds = nano / 1000 / 1000 / 1000;
  struct timespec     tv      = {.tv_sec  = seconds,
                        .tv_nsec = nano - seconds * 1000 * 1000 * 1000};
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "sleep for %ld ms", t);
  int ret = nanosleep(&tv, NULL);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Woke up");
  return ret;
}
