#define _XOPEN_SOURCE 500
#include "hostname.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <limits.h>
#include <unistd.h>

char* getHostName() {
  char* buf = secAlloc(HOST_NAME_MAX);
  if (gethostname(buf, HOST_NAME_MAX) != 0) {
    oidc_setErrnoError();
    secFree(buf);
    return NULL;
  }
  return buf;
}
