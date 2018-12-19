#include "errorUtils.h"

#include "stringUtils.h"

/**
 * combines an error and an error description string into one
 * @param error the error string
 * @param error_description the error description
 * @return a pointer to the combined string; has to be freed after usage.
 * @c NULL when both not valid
 */
char* combineError(const char* error, const char* error_description) {
  if (!strValid(error) && !strValid(error_description)) {
    return NULL;
  }
  if (!strValid(error_description)) {
    return oidc_strcopy(error);
  }
  return oidc_sprintf("%s: %s", error, error_description);
}
