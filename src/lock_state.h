#ifndef LOCK_STATE_H
#define LOCK_STATE_H

#include "crypt.h"
#include "oidc_error.h"

struct lock_state {
  short          locked;
  unsigned char* hashedPw;
  char           salt_hex[2 * SALT_LEN + 1];
};

oidc_error_t unlock(const char* password);
oidc_error_t lock(const char* password);

#endif  // LOCK_STATE_H
