#ifndef AGENT_STATE_H
#define AGENT_STATE_H

#include <time.h>

struct agent_state {
  time_t defaultTimeout;
  short  locked;
} agent_state;

#endif  // AGENT_STATE_H
