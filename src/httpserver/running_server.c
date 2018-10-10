#include "running_server.h"

#include "../utils/memory.h"

#include <string.h>

void secFreeRunningServer(struct running_server* s) {
  secFree(s->state);
  secFree(s);
}

int matchRunningServer(char* state, struct running_server* s) {
  return strcmp(s->state, state) == 0 ? 1 : 0;
}
