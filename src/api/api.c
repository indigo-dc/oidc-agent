#include "api.h"
#include "defines/ipc_values.h"
#ifdef __MINGW32__
#include "ipc/windows/cryptCommunicator.h"
#else
#include "ipc/cryptCommunicator.h"
#endif
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <stdlib.h>

#ifndef API_LOGLEVEL
#define API_LOGLEVEL NOTICE
#endif  // API_LOGLEVEL

#ifndef START_APILOGLEVEL
#define START_APILOGLEVEL int oldLogMask = logger_setloglevel(API_LOGLEVEL);
#endif
#ifndef END_APILOGLEVEL
#define END_APILOGLEVEL logger_setlogmask(oldLogMask);
#endif  // END_APILOGLEVEL


char* communicate(unsigned char remote, const char* fmt, ...) {
  START_APILOGLEVEL
  if (fmt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  va_list args;
  va_start(args, fmt);

  char* ret = ipc_vcryptCommunicate(remote, fmt, args);
  va_end(args);
  END_APILOGLEVEL
  return ret;
}

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }
