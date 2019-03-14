#ifndef LIFETIME_ARG_H
#define LIFETIME_ARG_H

#include <time.h>

struct lifetimeArg {
  time_t lifetime;
  short  argProvided;
};

#endif  // LIFETIME_ARG_H
