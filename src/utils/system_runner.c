#define _XOPEN_SOURCE
#include "system_runner.h"
#include "utils/file_io/file_io.h"
#include "utils/oidc_error.h"

#include <stdio.h>
#include <stdlib.h>
#include "utils/logger.h"

char* getOutputFromCommand(const char* cmd) {
  if (cmd == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  /* Open the command for reading. */
  FILE* fp = popen(cmd, "r");
  if (fp == NULL) {
    oidc_setErrnoError();
    logger(ERROR, "Failed to execute command: %s", cmd);
    return NULL;
  }

  char* ret = getLineFromFILE(fp);
  pclose(fp);
  return ret;
}
