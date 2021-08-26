#define _XOPEN_SOURCE
#include "system_runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/file_io/file_io.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

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

void fireCommand(const char* cmd) {
  pid_t pid = fork();
  if (pid == -1) {
    logger(ERROR, "fork %m");
    return;
  } else if (pid > 0) {  // parent
    return;
  }
  // child
  execlp("/bin/sh", "sh", "-c", cmd, (char*)NULL);
  /* exec functions only return on error */
  logger(ERROR, "Error executing command: %m");
  exit(EXIT_FAILURE);
}