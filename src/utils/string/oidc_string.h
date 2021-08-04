#ifndef OIDC_STRING_H
#define OIDC_STRING_H

#include <stddef.h>

#include "utils/oidc_error.h"

struct string {
  char*  ptr;
  size_t len;
};

oidc_error_t init_string(struct string* s);

#endif  // OIDC_STRING_H
