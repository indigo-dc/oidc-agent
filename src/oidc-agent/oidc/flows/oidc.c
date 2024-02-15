#include "oidc.h"

#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "account/account.h"
#include "defines/mytoken_values.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
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
  char* val  = urlescape((char*)list_at(list, 1)->val);
  char* data = oidc_sprintf("%s=%s", (char*)list_at(list, 0)->val, val);
  secFree(val);
  for (size_t i = 2; i < list->len - 1; i += 2) {
    val = urlescape((char*)list_at(list, i + 1)->val);
    char* tmp =
        oidc_sprintf("%s&%s=%s", data, (char*)list_at(list, i)->val, val);
    secFree(val);
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
  INIT_KEY_VALUE(OIDC_KEY_ACCESSTOKEN, OIDC_KEY_SCOPE, OIDC_KEY_REFRESHTOKEN,
                 MYTOKEN_KEY_MYTOKEN, OIDC_KEY_IDTOKEN, OIDC_KEY_EXPIRESIN,
                 OIDC_KEY_ERROR, OIDC_KEY_ERROR_DESCRIPTION);
  if (CALL_GETJSONVALUES(res) < 0) {
    agent_log(ERROR, "Error while parsing json\n");
    SEC_FREE_KEY_VALUES();
    if (oidc_errno == OIDC_EJSONPARS) {
      oidc_errno = OIDC_EOPNOJSON;
    }
    return NULL;
  }
  KEY_VALUE_VARS(access_token, scope, refresh_token, mytoken, id_token,
                 expires_in, error, error_description);
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
  if (strValid(_scope) && !refreshFlow) {
    // if we get a scope value back from the OP when the initial AT is obtained,
    // we update the config, because it might be possible that the OP made
    // changes to the scopes.
    // We use this updated scope value in future refresh request. For a
    // reauthenticate we will use the initial authorization scopes.
    account_setRefreshScope(a, _scope);
  } else {
    secFree(_scope);
  }

  char* refresh_token = account_getRefreshToken(a);
  char* obtainedRTMT  = _refresh_token ?: _mytoken;
  if (_refresh_token) {
    secFree(_mytoken);
  }
  if (strValid(obtainedRTMT) && !strequal(refresh_token, obtainedRTMT)) {
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
                account_getName(a), refresh_token, obtainedRTMT);
      oidcd_handleUpdateRefreshToken(pipes, account_getName(a), obtainedRTMT);
    }
    account_setRefreshToken(a, obtainedRTMT);
  } else {
    secFree(_refresh_token);
    secFree(_mytoken);
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

void addAudienceRFC8707ToList(list_t* postDataList, char* audience_cpy) {
  if (audience_cpy == NULL || postDataList == NULL) {
    return;
  }
  char* a = strtok(audience_cpy, " ");
  while (a) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_RESOURCE));
    list_rpush(postDataList, list_node_new(a));
    a = strtok(NULL, " ");
  }
}