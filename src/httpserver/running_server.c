#include "running_server.h"

#include "../oidc_utilities.h"

#include <string.h>

void clearFreeRunningServer(struct running_server* s) {
  clearFreeString(s->state);
  clearFree(s, sizeof(struct running_server));
}

int matchRunningServer(char* state, struct running_server* s) {
  return strcmp(s->state, state) == 0 ? 1 : 0;
}

