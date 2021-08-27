#ifndef DEATH_UTILS_H
#define DEATH_UTILS_H

#include <time.h>

#include "wrapper/list.h"

time_t getMinDeathFrom(list_t*, time_t (*)(void*));
void*  getDeathElementFrom(list_t*, time_t (*)(void*));

#endif  // DEATH_UTILS_H
