#ifndef RUNNING_SERVER_H
#define RUNNING_SERVER_H

#include <sys/types.h>

struct running_server {
  pid_t pid;
  char* state;
};

void clearFreeRunningServer(struct running_server* s);
int  matchRunningServer(char* state, struct running_server* s);

#endif  // RUNNING_SERVER_H
