#include "defines/settings.h"
#include "utils/agentLogger.h"
#include "utils/printer.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void sig_handler(int signo) {
  switch (signo) {
    case SIGSEGV: agent_log(EMERGENCY, "Caught Signal SIGSEGV"); break;
    default: agent_log(EMERGENCY, "Caught Signal %d", signo);
  }
  exit(signo);
}

void daemonize() {
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
    printStdout("%s=%d; export %s;\n", OIDC_PID_ENV_NAME, pid,
                OIDC_PID_ENV_NAME);
    printStdout("echo Agent pid $%s\n", OIDC_PID_ENV_NAME);
    exit(EXIT_SUCCESS);
  }
  chdir("/");
  umask(0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);
}
