#include "errorUtils.h"

#include "utils/memory.h"
#include "utils/string/stringUtils.h"

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

/**
 * checks if an error message matches a specific error code
 * handles both old format (error starts with errorcode) and new format
 * (error contains ": errorcode" after context like OP URL)
 * @param error the error message string
 * @param errorcode the error code to match
 * @return 1 if matched, 0 otherwise
 */
int matchError(const char* error, const char* errorcode) {
  if (error == NULL || errorcode == NULL) {
    return 0;
  }
  if (strstarts(error, errorcode)) {
    return 1;
  }
  char* pattern = oidc_sprintf(": %s", errorcode);
  int res       = strSubString(error, pattern);
  secFree(pattern);
  return res;
}
