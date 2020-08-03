#ifndef AGENT_STATE_H
#define AGENT_STATE_H

#include "lock_state.h"

#include <time.h>

struct agent_state {
  time_t            defaultTimeout;
  struct lock_state lock_state;
};

extern struct agent_state agent_state;

#endif  // AGENT_STATE_H
