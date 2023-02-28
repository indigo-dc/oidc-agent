#include "daemonize.h"

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils/agentLogger.h"

pid_t daemonize() {
  fflush(stdout);  // flush before forking, otherwise the buffered content is
                   // printed multiple times. (Only relevant when stdout is
                   // redirected, tty is line-buffered.)
  fflush(stderr);
  pid_t pid;
  if ((pid = fork()) == -1) {
    agent_log(ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }
  signal(SIGHUP, SIG_IGN);
  if ((pid = fork()) == -1) {
    agent_log(ALERT, "fork %m");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    return pid;
  }
  if (chdir("/") != 0) {
    agent_log(ERROR, "chdir %m");
  }
  umask(0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  assert(open("/dev/null", O_RDONLY) == STDIN_FILENO);
  assert(open("/dev/null", O_RDWR) == STDOUT_FILENO);
  assert(open("/dev/null", O_RDWR) == STDERR_FILENO);
  return pid;
}
