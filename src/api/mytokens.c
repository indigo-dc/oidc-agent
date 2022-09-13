#include "mytokens.h"

#include "api_helper.h"
#include "comm.h"
#include "defines/ipc_values.h"
#include "utils/errorUtils.h"
#include "utils/json.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

struct agent_response parseForMytokenAgentResponse(char* response) {
  struct agent_response res;
  if (response == NULL) {
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        oidc_strcopy("did not receive any response"), NULL};
    return res;
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, OIDC_KEY_ERROR_DESCRIPTION,
                 IPC_KEY_INFO, MYTOKEN_KEY_MYTOKEN, MYTOKEN_KEY_TRANSFERCODE,
                 MYTOKEN_KEY_TOKENTYPE, IPC_KEY_MYTOKEN_MY_ISS,
                 IPC_KEY_MYTOKEN_OIDC_ISS, MYTOKEN_KEY_RESTRICTIONS,
                 MYTOKEN_KEY_CAPABILITIES,
                 MYTOKEN_KEY_ROTATION, AGENT_KEY_EXPIRESAT);
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
  KEY_VALUE_VARS(status, error, error_description, info, mytoken, transfer_code,
                 mytoken_type, mytoken_iss, oidc_iss, restrictions,
                 capabilities,  rotation, expires_at);
  if (_error) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(_error);
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        combineError(_error, _error_description), oidc_strcopy(_info)};
    SEC_FREE_KEY_VALUES();
    return res;
  }
  secFree(_status);
  secFree(_info);
  oidc_errno        = OIDC_SUCCESS;
  time_t expires_at = strToLong(_expires_at);
  secFree(_expires_at);
  res.type = AGENT_RESPONSE_TYPE_MYTOKEN;
  char* token;
  if (strequal(MYTOKEN_MYTOKENTYPE_TRANSFERCODE, _mytoken_type)) {
    token = _transfer_code;
    secFree(_mytoken);
  } else {
    token = _mytoken;
    secFree(_transfer_code);
  }
  res.mytoken_response =
      (struct mytoken_response){.token                 = token,
                                .token_type            = _mytoken_type,
                                .mytoken_issuer        = _mytoken_iss,
                                .oidc_issuer           = _oidc_iss,
                                .restrictions          = _restrictions,
                                .capabilities          = _capabilities,
                                .rotation              = _rotation,
                                .expires_at            = expires_at};
  return res;
}

char* getMytokenRequest(const char* accountname, const char* profile,
                        const char* hint) {
  START_APILOGLEVEL
  cJSON* json =
      generateJSONObject(IPC_KEY_REQUEST, cJSON_String, REQUEST_VALUE_MYTOKEN,
                         IPC_KEY_SHORTNAME, cJSON_String, accountname, NULL);
  if (strValid(profile)) {
    jsonAddStringValue(json, AGENT_KEY_MYTOKENPROFILE, profile);
  }
  if (strValid(hint)) {
    jsonAddStringValue(json, IPC_KEY_APPLICATIONHINT, hint);
  }
  char* ret = jsonToStringUnformatted(json);
  secFreeJson(json);
  logger(DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

struct agent_response _getAgentMytokenResponseFromRequest(
    unsigned char remote, const char* ipc_request) {
  char* response = communicate(remote, ipc_request);
  return parseForMytokenAgentResponse(response);
}

struct agent_response getAgentMytokenResponse(const char* accountname,
                                              const char* mytoken_profile,
                                              const char* application_hint) {
  START_APILOGLEVEL
  char* request =
      getMytokenRequest(accountname, mytoken_profile, application_hint);
  struct agent_response res =
      _getAgentMytokenResponseFromRequest(LOCAL_COMM, request);
  struct oidc_error_state* localError = saveErrorState();
  const unsigned char      remote     = _checkLocalResponseForRemote(res);
  if (remote) {
    struct agent_response remote_res =
        _getAgentMytokenResponseFromRequest(remote, request);
    if (remote_res.type == AGENT_RESPONSE_TYPE_ERROR) {
      restoreErrorState(localError);
      secFreeAgentResponse(remote_res);
    } else {
      secFreeAgentResponse(res);
      res = remote_res;
    }
  }
  secFreeErrorState(localError);
  secFree(request);
  END_APILOGLEVEL
  return res;
}

char* getMytoken(const char* accountname, const char* mytoken_profile,
                 const char* application_hint) {
  START_APILOGLEVEL
  struct agent_response response =
      getAgentMytokenResponse(accountname, mytoken_profile, application_hint);
  if (response.type != AGENT_RESPONSE_TYPE_MYTOKEN) {
    if (response.type == AGENT_RESPONSE_TYPE_ERROR) {
      oidc_errno = OIDC_EERROR;
      oidc_seterror(response.error_response.error);
    }
    secFreeAgentResponse(response);
    return NULL;
  }
  char* mytoken = oidc_strcopy(response.mytoken_response.token);
  secFreeAgentResponse(response);
  END_APILOGLEVEL
  return mytoken;
}
