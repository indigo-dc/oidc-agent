#ifndef RUNNING_SERVER_H
#define RUNNING_SERVER_H

#include "list/list.h"

#include <sys/types.h>

struct running_server {
  pid_t pid;
  char* state;
};

void _secFreeRunningServer(struct running_server* s);
int  matchRunningServer(char* state, struct running_server* s);

void  addServer(struct running_server* running_server);
pid_t removeServer(const char* state);

#ifndef secFreeRunningServer
#define secFreeRunningServer(ptr) \
  do {                            \
    _secFreeRunningServer((ptr)); \
    (ptr) = NULL;                 \
  } while (0)
#endif  // secFreeRunningServer

#endif  // RUNNING_SERVER_H
