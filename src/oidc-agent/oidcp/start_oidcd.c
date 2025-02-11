#define _XOPEN_SOURCE 500
#include "start_oidcd.h"

#include <signal.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <unistd.h>

#include "ipc/pipe.h"
#include "oidc-agent/oidcd/oidcd.h"
#include "utils/agentLogger.h"
#include "utils/config/issuerConfig.h"

struct ipcPipe startOidcd(const struct arguments* arguments) {
  getIssuerConfig(
      NULL);  // we use this to trigger a read, so the issuer list is loaded and
              // we do not need to read the file later in oidcd
  struct pipeSet pipes = ipc_pipe_init();
  if (pipes.pipe1.rx == -1) {
    agent_log(ERROR, "could not create pipes: %s", oidc_serror());
    exit(EXIT_FAILURE);
  }
  pid_t ppid_before_fork = getpid();
  pid_t pid              = fork();
  if (pid == -1) {
    agent_log(ERROR, "fork %m");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {  // child
#ifdef __linux__
    // init child so that it exists if parent (oidcp) is killed.
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      agent_log(ERROR, "prctl %m");
      exit(EXIT_FAILURE);
    }
#endif
    // test in case the original parent exited just before the prctl() call
    if (getppid() != ppid_before_fork) {
      agent_log(ERROR, "Parent died shortly after fork");
      exit(EXIT_FAILURE);
    }
    struct ipcPipe childPipes = toClientPipes(pipes);
    oidcd_main(childPipes, arguments);
    exit(EXIT_FAILURE);
  } else {  // parent
    struct ipcPipe parentPipes = toServerPipes(pipes);
    return parentPipes;
  }
}
