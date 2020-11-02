#define _XOPEN_SOURCE
#include "system_runner.h"
#include "utils/file_io/file_io.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <stdio.h>
#include <stdlib.h>

char* getOutputFromCommand(const char* cmd) {
  if (cmd == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  logger(DEBUG, "Running command: %s", cmd);
  /* Open the command for reading. */
  FILE* fp = popen(cmd, "r");
  if (fp == NULL) {
    oidc_setErrnoError();
    logger(ERROR, "Failed to execute command: %s", cmd);
    return NULL;
  }
  char* ret = readFILE(fp);
  pclose(fp);
  return ret;
}
