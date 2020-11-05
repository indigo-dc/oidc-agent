#ifndef __APPLE__
#define _XOPEN_SOURCE 500
#else
#define _XOPEN_SOURCE 600
#endif
#include "hostname.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <limits.h>
#include <unistd.h>
#ifndef HOST_NAME_MAX
#include <sys/param.h>
#ifdef MAXHOSTNAMELEN
#define HOST_NAME_MAX MAXHOSTNAMELEN - 1
#else
#define HOST_NAME_MAX 255
#endif
#endif

char* getHostName() {
  char* buf = secAlloc(HOST_NAME_MAX);
  if (gethostname(buf, HOST_NAME_MAX) != 0) {
    oidc_setErrnoError();
    secFree(buf);
    return NULL;
  }
  return buf;
}
