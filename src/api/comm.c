#include "comm.h"
#include "api_helper.h"
#include "ipc/cryptCommunicator.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <stdarg.h>
#include <stdlib.h>

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
