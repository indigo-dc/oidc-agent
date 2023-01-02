#define _XOPEN_SOURCE 500
#include "system_runner.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils/file_io/file_io.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

char** splitCmd(const char* program) {
  char*  s        = oidc_strcopy(program);
  char** split    = secAlloc(sizeof(char*) * strCountChar(program, ' '));
  split[0]        = s;
  size_t        i = 1;
  unsigned char insideQuotes = 0;
  for (; *s != '\0'; s++) {
    switch (*s) {
      case ' ':
        if (!insideQuotes) {
          *s = '\0';
          if (*(s + 1) == '"') {
            s++;
            insideQuotes = !insideQuotes;
          }
          split[i] = s + 1;
          i++;
        }
        break;
      case '"':
        if (s != split[0] && *(s - 1) != '\\') {
          if (insideQuotes) {
            *s = '\0';
          }
          insideQuotes = !insideQuotes;
        }
        break;
    }
  }
  split[i] = NULL;
  return split;
}

#if defined __MSYS__ || defined _WIN32
static struct pid {
  struct pid* next;
  FILE*       fp;
  pid_t       pid;
} * pidlist;

FILE* win_popen(const char* program, const char* type) {
  struct pid* cur;
  FILE*       iop;
  int         pdes[2], pid;

  if ((*type != 'r' && *type != 'w') || type[1]) {
    errno = EINVAL;
    return NULL;
  }

  if ((cur = malloc(sizeof(struct pid))) == NULL) {
    return NULL;
  }

  if (pipe(pdes) < 0) {
    free(cur);
    return NULL;
  }

  switch (pid = vfork()) {
    case -1: /* Error. */
      (void)close(pdes[0]);
      (void)close(pdes[1]);
      free(cur);
      return NULL;
    /* NOTREACHED */
    case 0: /* Child. */
      if (*type == 'r') {
        if (pdes[1] != STDOUT_FILENO) {
          (void)dup2(pdes[1], STDOUT_FILENO);
          (void)close(pdes[1]);
        }
        (void)close(pdes[0]);
      } else {
        if (pdes[0] != STDIN_FILENO) {
          (void)dup2(pdes[0], STDIN_FILENO);
          (void)close(pdes[0]);
        }
        (void)close(pdes[1]);
      }
      char** args = splitCmd(program);
      execvp(args[0], args);
      _exit(127);
      /* NOTREACHED */
  }

  /* Parent; assume fdopen can't fail. */
  if (*type == 'r') {
    iop = fdopen(pdes[0], type);
    (void)close(pdes[1]);
  } else {
    iop = fdopen(pdes[1], type);
    (void)close(pdes[0]);
  }

  /* Link into list of file descriptors. */
  cur->fp   = iop;
  cur->pid  = pid;
  cur->next = pidlist;
  pidlist   = cur;

  return (iop);
}
#endif

char* getOutputFromCommand(const char* cmd) {
  if (cmd == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  logger(DEBUG, "Running command: %s", cmd);
  /* Open the command for reading. */
#if defined __MSYS__ || defined _WIN32
  FILE* fp = win_popen(cmd, "r");
#else
  FILE* fp = popen(cmd, "r");
#endif
  if (fp == NULL) {
    oidc_setErrnoError();
    logger(ERROR, "Failed to execute command: %s", cmd);
    logger(ERROR, oidc_serror());
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
#if defined __MSYS__ || defined _WIN32
  char** args = splitCmd(cmd);
  execvp(args[0], args);
#else
  execlp("/bin/sh", "sh", "-c", cmd, (char*)NULL);
#endif
  /* exec functions only return on error */
  logger(ERROR, "Error executing command: %m");
  exit(EXIT_FAILURE);
}