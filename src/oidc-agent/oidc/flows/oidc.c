#include "oidc.h"
#include "account/account.h"
#include "oidc-agent/oidc/values.h"
#include "oidc-agent/oidcd/internal_request_handler.h"
#include "utils/errorUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

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
  list_t* list = list_new();
  list_rpush(list, list_node_new(k1));
  list_rpush(list, list_node_new(v1));
  char* s;
  while ((s = va_arg(args, char*)) != NULL) {
    list_rpush(list, list_node_new(s));
  }
  va_end(args);
  char* data = generatePostDataFromList(list);
  list_destroy(list);
  return data;
}

char* generatePostDataFromList(list_t* list) {
  if (list == NULL || list->len < 2) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* data = oidc_sprintf("%s=%s", (char*)list_at(list, 0)->val,
                            (char*)list_at(list, 1)->val);
  for (size_t i = 2; i < list->len - 1; i += 2) {
    char* tmp = oidc_sprintf("%s&%s=%s", data, (char*)list_at(list, i)->val,
                             (char*)list_at(list, i + 1)->val);
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

char* parseTokenResponseCallbacks(
    const char* res, struct oidc_account* a, int saveAccessToken,
    void (*errorHandling)(const char*, const char*), struct ipcPipe pipes) {
  struct key_value pairs[5];
  for (size_t i = 0; i < sizeof(pairs) / sizeof(*pairs); i++) {
    pairs[i].value = NULL;
  }
  pairs[0].key = OIDC_KEY_ACCESSTOKEN;
  pairs[1].key = OIDC_KEY_REFRESHTOKEN;
  pairs[2].key = OIDC_KEY_EXPIRESIN;
  pairs[3].key = OIDC_KEY_ERROR;
  pairs[4].key = OIDC_KEY_ERROR_DESCRIPTION;
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
    account_setTokenExpiresAt(a, time(NULL) + strToInt(pairs[2].value));
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "expires_at is: %lu\n",
           account_getTokenExpiresAt(a));
    secFree(pairs[2].value);
  }
  char* refresh_token = account_getRefreshToken(a);
  if (strValid(pairs[1].value) && !strequal(refresh_token, pairs[1].value)) {
    account_setRefreshToken(a, pairs[1].value);
    if (refresh_token) {  // only update, if the refresh token changes, not when
                          // it is initially obtained
      oidcd_handleUpdateRefreshToken(pipes, account_getName(a), pairs[1].value);
    }
  } else {
    secFree(pairs[1].value);
  }
  if (saveAccessToken) {
    account_setAccessToken(a, pairs[0].value);
  }
  return pairs[0].value;
}

char* parseTokenResponse(const char* res, struct oidc_account* a,
                         int saveAccessToken, struct ipcPipe pipes) {
  return parseTokenResponseCallbacks(res, a, saveAccessToken,
                                     &defaultErrorHandling, pipes);
}
