#ifndef OIDC_AGENT_DURATION_H
#define OIDC_AGENT_DURATION_H

#include <time.h>

time_t parseDuration(const char* str);
time_t parseTime(const char* str);

#endif  // OIDC_AGENT_DURATION_H
