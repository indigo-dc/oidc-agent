#include "oidc_string.h"

#include <stdlib.h>
#include <syslog.h>

oidc_error_t init_string(struct string* s) {
  s->len = 0;
  s->ptr = secAlloc(s->len + 1);

  if (s->ptr == NULL) {
    syslog(LOG_AUTHPRIV | LOG_EMERG, "%s (%s:%d) alloc() failed: %m\n",
           __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    return OIDC_EALLOC;
  }
  s->ptr[0] = '\0';
  return OIDC_SUCCESS;
}
