#ifndef LOCK_STATE_H
#define LOCK_STATE_H

#include "crypt.h"
#include "oidc_error.h"
#include "utils/cryptUtils.h"

struct lock_state {
  short          locked;
  struct hashed* hash;
};

void lock_state_setHash(struct lock_state* l, struct hashed* h) {
  secFree(l->hash);
  l->hash = h;
}

oidc_error_t unlock(const char* password);
oidc_error_t lock(const char* password);

#endif  // LOCK_STATE_H
