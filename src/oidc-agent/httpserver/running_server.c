#include "running_server.h"

#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/stringUtils.h"

#include <string.h>
#include <syslog.h>

static list_t* servers = NULL;

void _secFreeRunningServer(struct running_server* s) {
  secFree(s->state);
  secFree(s);
}

int matchRunningServer(char* state, struct running_server* s) {
  return strcmp(s->state, state) == 0 ? 1 : 0;
}

void addServer(struct running_server* running_server) {
  if (servers == NULL) {
    servers        = list_new();
    servers->free  = (void (*)(void*)) & _secFreeRunningServer;
    servers->match = (int (*)(void*, void*)) & matchRunningServer;
  }
  list_rpush(servers, list_node_new(running_server));
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Added Server. Now %d server run",
         servers->len);
}

pid_t removeServer(const char* state) {
  if (servers == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No servers running");
    return -1;
  }
  list_node_t* n = findInList(servers, (char*)state);
  if (n == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No server found for state %s", state);
    return -1;
  }
  pid_t pid = ((struct running_server*)n->val)->pid;
  list_remove(servers, n);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Removed Server. Now %d server run",
         servers->len);
  return pid;
}
