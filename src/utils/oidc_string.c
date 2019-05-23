#include "oidc_string.h"
#include "utils/logger.h"

#include <stdlib.h>

oidc_error_t init_string(struct string* s) {
  s->len = 0;
  s->ptr = secAlloc(s->len + 1);

  if (s->ptr == NULL) {
    logger(EMERGENCY, "%s (%s:%d) alloc() failed: %m\n", __func__, __FILE__,
           __LINE__);
    oidc_errno = OIDC_EALLOC;
    return OIDC_EALLOC;
  }
  return OIDC_SUCCESS;
}
