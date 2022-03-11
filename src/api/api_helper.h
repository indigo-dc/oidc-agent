#ifndef OIDC_AGENT_API_HELPER_H
#define OIDC_AGENT_API_HELPER_H

#include "utils/logger.h"

#ifndef API_LOGLEVEL
#define API_LOGLEVEL NOTICE
#endif  // API_LOGLEVEL

#ifndef START_APILOGLEVEL
#define START_APILOGLEVEL int oldLogMask = logger_setloglevel(API_LOGLEVEL);
#endif
#ifndef END_APILOGLEVEL
#define END_APILOGLEVEL logger_setlogmask(oldLogMask);
#endif  // END_APILOGLEVEL

#define LOCAL_COMM 0
#define REMOTE_COMM 1

#endif  // OIDC_AGENT_API_HELPER_H
