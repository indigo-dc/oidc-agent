#include "../account.h"
#include "../oidc_error.h"
#include "../utils/errorUtils.h"
#include "../utils/memory.h"
#include "../utils/stringUtils.h"

#include <stdarg.h>
#include <stddef.h>
#include <syslog.h>
#include <time.h>

/**
 * last argument has to be NULL
 */
char* generatePostData(char* k1, char* v1, ...) {
  va_list args;
  va_start(args, v1);

  char* data = oidc_sprintf("%s=%s", k1, v1);
  char* s;
  while ((s = va_arg(args, char*)) != NULL) {
    char* tmp = oidc_sprintf("%s&%s=%s", data, s, va_arg(args, char*));
    if (tmp == NULL) {
      return NULL;
    }
    secFree(data);
    data = tmp;
  }
  return data;
}

void defaultErrorHandling(const char* error, const char* error_description) {
  char* error_str = combineError(error, error_description);
  oidc_seterror(error_str);
  oidc_errno = OIDC_EOIDC;
  syslog(LOG_AUTHPRIV | LOG_ERR, "%s", error);
  secFree(error_str);
}

char* parseTokenResponseCallbacks(const char* res, struct oidc_account* a,
                                  int saveAccessToken, int saveRefreshToken,
                                  void (*errorHandling)(const char*,
                                                        const char*)) {
  struct key_value pairs[5];
  pairs[0].key   = "access_token";
  pairs[0].value = NULL;
  pairs[1].key   = "refresh_token";
  pairs[1].value = NULL;
  pairs[2].key   = "expires_in";
  pairs[2].value = NULL;
  pairs[3].key   = "error";
  pairs[3].value = NULL;
  pairs[4].key   = "error_description";
  pairs[4].value = NULL;
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(pairs[0])) <
      0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Error while parsing json\n");
    return NULL;
  }
  if (pairs[3].value || pairs[4].value) {
    errorHandling(pairs[3].value, pairs[4].value);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    return NULL;
  }
  if (NULL != pairs[2].value) {
    account_setTokenExpiresAt(a, time(NULL) + atoi(pairs[2].value));
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "expires_at is: %lu\n",
           account_getTokenExpiresAt(*a));
    secFree(pairs[2].value);
  }
  char* refresh_token = account_getRefreshToken(*a);
  if (!saveRefreshToken && strValid(pairs[1].value) &&
      strcmp(refresh_token, pairs[1].value) != 0) {
    syslog(LOG_AUTHPRIV | LOG_WARNING,
           "WARNING: Received new refresh token from OIDC Provider. It's most "
           "likely that the old one was therefore revoked. We did not save the "
           "new refresh token. You may want to revoke it. You have to run "
           "oidc-gen again.");
  }
  secFree(refresh_token);
  if (saveRefreshToken) {
    account_setRefreshToken(a, pairs[1].value);
  } else {
    secFree(pairs[1].value);
  }
  if (saveAccessToken) {
    account_setAccessToken(a, pairs[0].value);
  }
  return pairs[0].value;
}

char* parseTokenResponse(const char* res, struct oidc_account* a,
                         int saveAccessToken, int saveRefreshToken) {
  return parseTokenResponseCallbacks(res, a, saveAccessToken, saveRefreshToken,
                                     &defaultErrorHandling);
}
