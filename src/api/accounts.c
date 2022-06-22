#include "accounts.h"

#include "api_helper.h"
#include "comm.h"
#include "defines/ipc_values.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

struct agent_response parseForAgentLoadedAccountsListResponse(char* response) {
  struct agent_response res;
  if (response == NULL) {
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        oidc_strcopy("did not receive any response"), NULL};
    return res;
  }

  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, "info");
  if (CALL_GETJSONVALUES(response) < 0) {
    secFree(response);
    SEC_FREE_KEY_VALUES();
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        oidc_strcopy("read malformed data"),
        oidc_strcopy("Please hand in a bug report: "
                     "https://github.com/indigo-dc/oidc-agent")};
    return res;
  }

  secFree(response);
  KEY_VALUE_VARS(status, error, info);
  if (_error) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(_error);
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){oidc_strcopy(_error),
                                                       oidc_strcopy(_info)};
    SEC_FREE_KEY_VALUES();
    return res;
  } else {
    secFree(_status);
    oidc_errno                            = OIDC_SUCCESS;
    res.type                              = AGENT_RESPONSE_TYPE_ACCOUNTS;
    res.loaded_accounts_response.accounts = _info;
    return res;
  }
}

char* getLoadedAccountsListRequest() {
  START_APILOGLEVEL
  cJSON* json = generateJSONObject(IPC_KEY_REQUEST, cJSON_String,
                                   REQUEST_VALUE_LOADEDACCOUNTS, NULL);
  char*  ret  = jsonToStringUnformatted(json);
  secFreeJson(json);
  logger(DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

struct agent_response _getAgentLoadedAccountsListFromRequest(
    unsigned char remote, const char* ipc_request) {
  char* response = communicate(remote, ipc_request);
  return parseForAgentLoadedAccountsListResponse(response);
}

struct agent_response getAgentLoadedAccountsListResponse() {
  START_APILOGLEVEL
  char*                 request = getLoadedAccountsListRequest();
  struct agent_response ret =
      _getAgentLoadedAccountsListFromRequest(LOCAL_COMM, request);
  struct oidc_error_state* localError = saveErrorState();
  const unsigned char      remote     = _checkLocalResponseForRemote(ret);
  if (remote) {
    struct agent_response retRemote =
        _getAgentLoadedAccountsListFromRequest(remote, request);
    if (retRemote.type == AGENT_RESPONSE_TYPE_ERROR) {
      restoreErrorState(localError);
      secFreeAgentResponse(retRemote);
    } else {
      secFreeAgentResponse(ret);
      ret = retRemote;
    }
  }
  secFreeErrorState(localError);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}

char* getLoadedAccountsList() {
  struct agent_response response = getAgentLoadedAccountsListResponse();
  if (response.type == AGENT_RESPONSE_TYPE_ACCOUNTS) {
    return response.loaded_accounts_response.accounts;
  }
  secFreeAgentResponse(response);
  return NULL;
}
