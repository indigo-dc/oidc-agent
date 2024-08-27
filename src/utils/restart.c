#define _XOPEN_SOURCE 500
#include "restart.h"

#include <unistd.h>

#include "defines/settings.h"
#include "utils/agentLogger.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

static int    restart_argc;
static char** restart_argv;

void set_restart_agent_args(int argc, char* argv[]) {
  restart_argc = argc;
  restart_argv = argv;
}

void restart_agent_with_set_args() {
  restart_agent(restart_argc, restart_argv);
}

void restart_agent(int argc, char* argv[]) {
  // Rebuild the arguments array, ensuring the last element is NULL as required
  // by execv
  char* new_argv[argc + 1];
  for (int i = 0; i < argc; i++) { new_argv[i] = argv[i]; }
  new_argv[argc] = NULL;

  agent_log(INFO, "Restarting oidc-agent %s ...", AGENT_PATH);
  execv(AGENT_PATH, new_argv);

  // This is only reached in error case
  oidc_setErrnoError();
  agent_log(ERROR, "%s", oidc_serror());
}
