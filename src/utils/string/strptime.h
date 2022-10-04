#ifndef OIDC_AGENT_STRPTIME_H
#define OIDC_AGENT_STRPTIME_H

#include "defines/msys.h"
#ifdef MINGW
#include <time.h>

char * strptime(const char *buf, const char *fmt, struct tm *tm);

#endif
#endif  // OIDC_AGENT_STRPTIME_H
