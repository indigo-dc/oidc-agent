#ifndef LOCK_STATE_H
#define LOCK_STATE_H

#include "utils/oidc_error.h"

struct lock_state {
  short          locked;
  struct hashed* hash;
};

oidc_error_t unlock(const char* password);
oidc_error_t lock(const char* password);

#endif  // LOCK_STATE_H
