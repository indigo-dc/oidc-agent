#define _GNU_SOURCE
#include "termHttpserver.h"
#include "list/src/list.h"
#include "running_server.h"
#include "utils/memory.h"

#include <microhttpd.h>
#include <signal.h>
#include <sys/types.h>
#include <syslog.h>

void stopHttpServer(struct MHD_Daemon** d_ptr) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "HttpServer: Stopping HttpServer");
  MHD_stop_daemon(*d_ptr);
  secFree(d_ptr);
}

void termHttpServer(const char* state) {
  if (state == NULL) {
    return;
  }
  if (servers == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No servers running");
    return;
  }
  list_node_t* n = list_find(servers, (char*)state);
  if (n == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No server found for state %s", state);
    return;
  }
  kill(((struct running_server*)n->val)->pid, SIGTERM);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "killed webserver for state %s with pid %d",
         state, ((struct running_server*)n->val)->pid);
  list_remove(servers, n);
}
