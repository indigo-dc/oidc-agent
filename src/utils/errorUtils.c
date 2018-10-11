#include "errorUtils.h"

#include "stringUtils.h"

char* combineError(const char* error, const char* error_description) {
  if (!strValid(error) && !strValid(error_description)) {
    return NULL;
  }
  if (!strValid(error_description)) {
    return oidc_strcopy(error);
  }
  return oidc_sprintf("%s: %s", error, error_description);
}
