#include "oidc.h"

#include <stdarg.h>
#include <stddef.h>
#include <time.h>

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/oidcd/internal_request_handler.h"
#include "utils/agentLogger.h"
#include "utils/errorUtils.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

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
  agent_log(ERROR, "%s", error);
  secFree(error_str);
}

#define TOKENPARSEMODE_DONTFREE_AT \
  (TOKENPARSEMODE_SAVE_AT | TOKENPARSEMODE_RETURN_AT)
#define TOKENPARSEMODE_DONTFREE_ID TOKENPARSEMODE_RETURN_ID

char* parseTokenResponseCallbacks(
    const unsigned char mode, const char* res, struct oidc_account* a,
    void (*errorHandling)(const char*, const char*), struct ipcPipe pipes,
    const unsigned char refreshFlow) {
  if (mode & TOKENPARSEMODE_RETURN_ID && mode & TOKENPARSEMODE_RETURN_AT) {
    oidc_setInternalError("cannot return AT and ID token");
    return NULL;
  }
  INIT_KEY_VALUE(OIDC_KEY_ACCESSTOKEN, OIDC_KEY_REFRESHTOKEN, OIDC_KEY_IDTOKEN,
                 OIDC_KEY_EXPIRESIN, OIDC_KEY_ERROR,
                 OIDC_KEY_ERROR_DESCRIPTION);
  if (CALL_GETJSONVALUES(res) < 0) {
    agent_log(ERROR, "Error while parsing json\n");
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  KEY_VALUE_VARS(access_token, refresh_token, id_token, expires_in, error,
                 error_description);
  if (_error || _error_description) {
    errorHandling(_error, _error_description);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }

  if (NULL != _expires_in) {
    if (mode & TOKENPARSEMODE_SAVE_AT) {
      account_setTokenExpiresAt(a, time(NULL) + strToInt(_expires_in));
      agent_log(DEBUG, "expires_at is: %lu\n", account_getTokenExpiresAt(a));
    }
    secFree(_expires_in);
  }

  char* refresh_token = account_getRefreshToken(a);
  if (strValid(_refresh_token) && !strequal(refresh_token, _refresh_token)) {
    if (strValid(refresh_token) &&
        refreshFlow) {  // only update, if the refresh token
                        // changes, not when
                        // it is initially obtained
                        // Also only update when doing the
                        // refresh flow, on other flows the refresh
                        // token did not change in the same sense.
                        // It's possible that an RT "changes" if
                        // performing the auth code flow or another
                        // flow but the user provided an RT (e.g. in
                        // a file), but it wasn't used. (Unlikely,
                        // but possible)
      agent_log(DEBUG, "Updating refreshtoken for %s from '%s' to '%s'",
                account_getName(a), refresh_token, _refresh_token);
      oidcd_handleUpdateRefreshToken(pipes, account_getName(a), _refresh_token);
    }
    account_setRefreshToken(a, _refresh_token);
  } else {
    secFree(_refresh_token);
  }

  if (mode & TOKENPARSEMODE_SAVE_AT) {
    account_setAccessToken(a, _access_token);
  }

  if (!(mode & TOKENPARSEMODE_DONTFREE_AT)) {
    secFree(_access_token);
  }
  if (!(mode & TOKENPARSEMODE_DONTFREE_ID)) {
    secFree(_id_token);
  }

  if (mode & TOKENPARSEMODE_RETURN_AT) {
    return _access_token;
  } else if (mode & TOKENPARSEMODE_RETURN_ID) {
    return _id_token;
  }
  oidc_errno = OIDC_SUCCESS;
  return NULL;
}

char* parseTokenResponse(const unsigned char mode, const char* res,
                         struct oidc_account* a, struct ipcPipe pipes,
                         const unsigned char refreshFlow) {
  return parseTokenResponseCallbacks(mode, res, a, &defaultErrorHandling, pipes,
                                     refreshFlow);
}
