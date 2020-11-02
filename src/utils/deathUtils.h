#ifndef DEATH_UTILS_H
#define DEATH_UTILS_H

#include "wrapper/list.h"

#include <time.h>

time_t getMinDeathFrom(list_t*, time_t (*)(void*));
void*  getDeathElementFrom(list_t*, time_t (*)(void*));

#endif  // DEATH_UTILS_H
