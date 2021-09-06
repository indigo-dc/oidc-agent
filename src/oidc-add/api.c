#include "api.h"
#include "api/api.h"
#include "defines/ipc_values.h"

#include "parse.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <stdarg.h>
#include <stdlib.h>

#ifndef API_LOGLEVEL
#define API_LOGLEVEL NOTICE
#endif  // API_LOGLEVEL

#ifndef START_APILOGLEVEL
#define START_APILOGLEVEL int oldLogMask = logger_setloglevel(API_LOGLEVEL);
#endif
#ifndef END_APILOGLEVEL
#define END_APILOGLEVEL logger_setlogmask(oldLogMask);
#endif  // END_APILOGLEVEL

#define LOCAL_COMM 0
#define REMOTE_COMM 1

unsigned char _checkLocalResponseForRemote(struct loaded_accounts_response res) {
  if (res.accounts != NULL) {
    return LOCAL_COMM;
  }
  const char* err = oidc_serror();
  if (strstarts(err, "Could not connect to oidc-agent") ||
      strequal(err, "OIDC_SOCK env var not set")) {
    return REMOTE_COMM;
  }
  return LOCAL_COMM;
}

char* getLoadedAccountsListRequest() {
  START_APILOGLEVEL
  cJSON* json = generateJSONObject(IPC_KEY_REQUEST, cJSON_String, REQUEST_VALUE_LOADEDACCOUNTS, NULL);
  char* ret = jsonToStringUnformatted(json);
  secFreeJson(json);
  logger(DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

struct loaded_accounts_response _getLoadedAccountsListFromRequest(unsigned char remote,
                                                   const char*   ipc_request) {
  char* response = communicate(remote, ipc_request);
  return parseForLoadedAccountsListResponse(response);
}

struct loaded_accounts_response getLoadedAccountsListResponse() {
  START_APILOGLEVEL
  char* request = getLoadedAccountsListRequest();
  struct loaded_accounts_response ret = _getLoadedAccountsListFromRequest(LOCAL_COMM, request);
  struct oidc_error_state* localError = saveErrorState();
  const unsigned char      remote     = _checkLocalResponseForRemote(ret);
  if (remote) {
    ret = _getLoadedAccountsListFromRequest(remote, request);
    if (ret.accounts == NULL) {
      restoreErrorState(localError);
    }
  }
  secFreeErrorState(localError);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}
char* getLoadedAccountsList() {
  START_APILOGLEVEL
  struct loaded_accounts_response response = getLoadedAccountsListResponse();
  END_APILOGLEVEL
  return response.accounts;
}

void secFreeLoadedAccountsListResponse(struct loaded_accounts_response response) {
  START_APILOGLEVEL
  secFree(response.accounts);
  END_APILOGLEVEL
}
