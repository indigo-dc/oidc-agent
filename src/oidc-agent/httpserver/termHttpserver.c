#define _POSIX_C_SOURCE 200809L
#include "termHttpserver.h"
#include "running_server.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"

#include <microhttpd.h>
#include <signal.h>
#include <sys/types.h>

void stopHttpServer(struct MHD_Daemon** d_ptr) {
  agent_log(DEBUG, "HttpServer: Stopping HttpServer");
  MHD_stop_daemon(*d_ptr);
  secFree(d_ptr);
}

void termHttpServer(const char* state) {
  if (state == NULL) {
    return;
  }
  pid_t pid = removeServer(state);
  if (pid > 0) {
    kill(pid, SIGTERM);
    agent_log(DEBUG, "killed webserver for state %s with pid %d", state, pid);
  }
}
