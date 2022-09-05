#include "tokens.h"

#include "api_helper.h"
#include "comm.h"
#include "defines/ipc_values.h"
#include "utils/errorUtils.h"
#include "utils/json.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"
struct agent_response parseForAgentResponse(char* response) {
  struct agent_response res;
  if (response == NULL) {
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        oidc_strcopy("did not receive any response"), NULL};
    oidc_errno = OIDC_EERROR;
    oidc_seterror(res.error_response.error);
    return res;
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, OIDC_KEY_ERROR_DESCRIPTION,
                 IPC_KEY_INFO, OIDC_KEY_ACCESSTOKEN, OIDC_KEY_ISSUER,
                 AGENT_KEY_EXPIRESAT);
  if (CALL_GETJSONVALUES(response) < 0) {
    secFree(response);
    SEC_FREE_KEY_VALUES();
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        oidc_strcopy("read malformed data"),
        oidc_strcopy("Please hand in a bug report: "
                     "https://github.com/indigo-dc/oidc-agent")};
    oidc_errno = OIDC_EERROR;
    oidc_seterror(res.error_response.error);
    return res;
  }
  secFree(response);
  KEY_VALUE_VARS(status, error, error_description, info, access_token, issuer,
                 expires_at);
  if (_error) {  // error
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        combineError(_error, _error_description), oidc_strcopy(_info)};
    oidc_errno = OIDC_EERROR;
    oidc_seterror(res.error_response.error);
    SEC_FREE_KEY_VALUES();
    return res;
  } else {
    secFree(_status);
    oidc_errno        = OIDC_SUCCESS;
    time_t expires_at = strToLong(_expires_at);
    secFree(_expires_at);
    res.type = AGENT_RESPONSE_TYPE_TOKEN;
    res.token_response =
        (struct token_response){_access_token, _issuer, expires_at};
    return res;
  }
}

struct token_response parseForTokenResponse(char* response) {
  struct agent_response agentResponse = parseForAgentResponse(response);
  if (agentResponse.type == AGENT_RESPONSE_TYPE_TOKEN) {
    return agentResponse.token_response;
  }
  secFreeAgentResponse(agentResponse);
  return (struct token_response){NULL, NULL, 0};
}

char* _getAccessTokenRequest(const char* accountname, const char* issuer,
                             time_t min_valid_period, const char* scope,
                             const char* hint, const char* audience) {
  START_APILOGLEVEL
  cJSON* json = generateJSONObject(IPC_KEY_REQUEST, cJSON_String,
                                   REQUEST_VALUE_ACCESSTOKEN, IPC_KEY_MINVALID,
                                   cJSON_Number, min_valid_period, NULL);
  if (strValid(accountname)) {
    jsonAddStringValue(json, IPC_KEY_SHORTNAME, accountname);
  } else if (strValid(issuer)) {
    jsonAddStringValue(json, IPC_KEY_ISSUERURL, issuer);
  }
  if (strValid(scope)) {
    jsonAddStringValue(json, OIDC_KEY_SCOPE, scope);
  }
  if (strValid(hint)) {
    jsonAddStringValue(json, IPC_KEY_APPLICATIONHINT, hint);
  }
  if (strValid(audience)) {
    jsonAddStringValue(json, IPC_KEY_AUDIENCE, audience);
  }
  char* ret = jsonToStringUnformatted(json);
  secFreeJson(json);
  logger(DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

char* getAccessTokenRequest(const char* accountname, time_t min_valid_period,
                            const char* scope, const char* hint,
                            const char* audience) {
  return _getAccessTokenRequest(accountname, NULL, min_valid_period, scope,
                                hint, audience);
}

char* getAccessTokenRequestIssuer(const char* issuer, time_t min_valid_period,
                                  const char* scope, const char* hint,
                                  const char* audience) {
  return _getAccessTokenRequest(NULL, issuer, min_valid_period, scope, hint,
                                audience);
}

struct agent_response _getAgentResponseFromRequest(unsigned char remote,
                                                   const char*   ipc_request) {
  char* response = communicate(remote, ipc_request);
  return parseForAgentResponse(response);
}

struct token_response _agentResponseToTokenResponse(
    struct agent_response agentResponse) {
  if (agentResponse.type == AGENT_RESPONSE_TYPE_TOKEN) {
    return agentResponse.token_response;
  }
  secFreeAgentResponse(agentResponse);
  return (struct token_response){NULL, NULL, 0};
}

struct token_response getTokenResponse(const char* accountname,
                                       time_t      min_valid_period,
                                       const char* scope,
                                       const char* application_hint) {
  return getTokenResponse3(accountname, min_valid_period, scope,
                           application_hint, NULL);
}

struct token_response getTokenResponse3(const char* accountname,
                                        time_t      min_valid_period,
                                        const char* scope,
                                        const char* application_hint,
                                        const char* audience) {
  START_APILOGLEVEL
  struct agent_response res = getAgentTokenResponse(
      accountname, min_valid_period, scope, application_hint, audience);
  struct token_response ret = _agentResponseToTokenResponse(res);
  END_APILOGLEVEL
  return ret;
}

struct agent_response getAgentTokenResponse(const char* accountname,
                                            time_t      min_valid_period,
                                            const char* scope,
                                            const char* application_hint,
                                            const char* audience) {
  START_APILOGLEVEL
  char* request = getAccessTokenRequest(accountname, min_valid_period, scope,
                                        application_hint, audience);
  struct agent_response res = _getAgentResponseFromRequest(LOCAL_COMM, request);
  struct oidc_error_state* localError = saveErrorState();
  const unsigned char      remote     = _checkLocalResponseForRemote(res);
  if (remote) {
    struct agent_response remote_res =
        _getAgentResponseFromRequest(remote, request);
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

struct token_response getTokenResponseForIssuer(const char* issuer_url,
                                                time_t      min_valid_period,
                                                const char* scope,
                                                const char* application_hint) {
  return getTokenResponseForIssuer3(issuer_url, min_valid_period, scope,
                                    application_hint, NULL);
}

struct token_response getTokenResponseForIssuer3(const char* issuer_url,
                                                 time_t      min_valid_period,
                                                 const char* scope,
                                                 const char* application_hint,
                                                 const char* audience) {
  START_APILOGLEVEL
  struct agent_response res = getAgentTokenResponseForIssuer(
      issuer_url, min_valid_period, scope, application_hint, audience);
  struct token_response ret = _agentResponseToTokenResponse(res);
  END_APILOGLEVEL
  return ret;
}

struct agent_response getAgentTokenResponseForIssuer(
    const char* issuer_url, time_t min_valid_period, const char* scope,
    const char* application_hint, const char* audience) {
  START_APILOGLEVEL
  char* request = getAccessTokenRequestIssuer(
      issuer_url, min_valid_period, scope, application_hint, audience);
  struct agent_response res = _getAgentResponseFromRequest(LOCAL_COMM, request);
  secFree(request);
  END_APILOGLEVEL
  return res;
}

char* getAccessToken(const char* accountname, time_t min_valid_period,
                     const char* scope) {
  return getAccessToken2(accountname, min_valid_period, scope, NULL);
}

char* getAccessToken2(const char* accountname, time_t min_valid_period,
                      const char* scope, const char* application_hint) {
  return getAccessToken3(accountname, min_valid_period, scope, application_hint,
                         NULL);
}

char* getAccessToken3(const char* accountname, time_t min_valid_period,
                      const char* scope, const char* application_hint,
                      const char* audience) {
  START_APILOGLEVEL
  struct token_response response = getTokenResponse3(
      accountname, min_valid_period, scope, application_hint, audience);
  secFree(response.issuer);
  END_APILOGLEVEL
  return response.token;
}

char* getAccessTokenForIssuer(const char* issuer_url, time_t min_valid_period,
                              const char* scope, const char* application_hint) {
  START_APILOGLEVEL
  struct token_response response = getTokenResponseForIssuer3(
      issuer_url, min_valid_period, scope, application_hint, NULL);
  secFree(response.issuer);
  END_APILOGLEVEL
  return response.token;
}

char* getAccessTokenForIssuer3(const char* issuer_url, time_t min_valid_period,
                               const char* scope, const char* application_hint,
                               const char* audience) {
  START_APILOGLEVEL
  struct token_response response = getTokenResponseForIssuer3(
      issuer_url, min_valid_period, scope, application_hint, audience);
  secFree(response.issuer);
  END_APILOGLEVEL
  return response.token;
}
