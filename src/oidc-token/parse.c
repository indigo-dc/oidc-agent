#include "parse.h"

#include "api.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/string/stringUtils.h"

struct agent_response parseForAgentResponse(char* response) {
  struct agent_response res;
  if (response == NULL) {
    res.type           = AGENT_RESPONSE_TYPE_ERROR;
    res.error_response = (struct agent_error_response){
        oidc_strcopy("did not receive any response"), NULL};
    return res;
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_INFO,
                 OIDC_KEY_ACCESSTOKEN, OIDC_KEY_ISSUER, AGENT_KEY_EXPIRESAT);
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
  KEY_VALUE_VARS(status, error, info, access_token, issuer, expires_at);
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
    oidc_errno        = OIDC_SUCCESS;
    time_t expires_at = strToULong(_expires_at);
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
