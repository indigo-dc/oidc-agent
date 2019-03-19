#include "api.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "ipc/communicator.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"

#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>

#ifndef API_LOGLEVEL
#define API_LOGLEVEL LOG_NOTICE
#endif  // API_LOGLEVEL

#ifndef START_APILOGLEVEL
#define START_APILOGLEVEL int oldLogMask = setlogmask(LOG_UPTO(API_LOGLEVEL));
#endif
#ifndef END_APILOGLEVEL
#define END_APILOGLEVEL setlogmask(oldLogMask);
#endif  // END_APILOGLEVEL

char* getAccessTokenRequest(const char* accountname, time_t min_valid_period,
                            const char* scope, const char* hint) {
  START_APILOGLEVEL
  cJSON* json = generateJSONObject(IPC_KEY_REQUEST, cJSON_String,
                                   REQUEST_VALUE_ACCESSTOKEN, IPC_KEY_SHORTNAME,
                                   cJSON_String, accountname, IPC_KEY_MINVALID,
                                   cJSON_Number, min_valid_period, NULL);
  if (strValid(scope)) {
    jsonAddStringValue(json, OIDC_KEY_SCOPE, scope);
  }
  if (strValid(hint)) {
    jsonAddStringValue(json, IPC_KEY_APPLICATIONHINT, hint);
  }
  char* ret = jsonToStringUnformatted(json);
  secFreeJson(json);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

char* getAccessTokenRequestIssuer(const char* issuer, time_t min_valid_period,
                                  const char* scope,
                                  const char* hint) {  // TODO refactor
  START_APILOGLEVEL
  cJSON*json = generateJSONObject(IPC_KEY_REQUEST, cJSON_String,
                                   REQUEST_VALUE_ACCESSTOKEN, IPC_KEY_ISSUERURL,
                                   cJSON_String, issuer, IPC_KEY_MINVALID,
                                   cJSON_Number, min_valid_period, NULL);
  if (strValid(scope)) {
    jsonAddStringValue(json, OIDC_KEY_SCOPE, scope);
  }
  if (strValid(hint)) {
    jsonAddStringValue(json, IPC_KEY_APPLICATIONHINT, hint);
  }
  char*ret = jsonToStringUnformatted(json);
  secFreeJson(json);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

char* communicate(const char* fmt, ...) {
  START_APILOGLEVEL
  if (fmt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  va_list args;
  va_start(args, fmt);

  char* ret = ipc_vcommunicate(fmt, args);
  va_end(args);
  END_APILOGLEVEL
  return ret;
}

struct token_response _getTokenResponseFromRequest(const char* ipc_request) {
  char* response = communicate(ipc_request);
  if (response == NULL) {
    return (struct token_response){NULL, NULL, 0};
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, OIDC_KEY_ACCESSTOKEN,
                 OIDC_KEY_ISSUER, AGENT_KEY_EXPIRESAT);
  if (CALL_GETJSONVALUES(response) < 0) {
    printError("Read malformed data. Please hand in bug report.\n");
    secFree(response);
    SEC_FREE_KEY_VALUES();
    return (struct token_response){NULL, NULL, 0};
  }
  secFree(response);
  KEY_VALUE_VARS(status, error, access_token, issuer, expires_at);
  if (_error) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(_error);
    SEC_FREE_KEY_VALUES();
    return (struct token_response){NULL, NULL, 0};
  } else {
    secFree(_status);
    oidc_errno        = OIDC_SUCCESS;
    time_t expires_at = strToULong(_expires_at);
    secFree(_expires_at);
    return (struct token_response){_access_token, _issuer, expires_at};
  }
}

struct token_response getTokenResponse(const char* accountname,
                                       time_t      min_valid_period,
                                       const char* scope,
                                       const char* application_hint) {
  START_APILOGLEVEL
  char* request = getAccessTokenRequest(accountname, min_valid_period, scope,
                                        application_hint);
  struct token_response ret = _getTokenResponseFromRequest(request);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}

struct token_response getTokenResponseForIssuer(const char* issuer_url,
                                                time_t      min_valid_period,
                                                const char* scope,
                                                const char* application_hint) {
  START_APILOGLEVEL
  char* request = getAccessTokenRequestIssuer(issuer_url, min_valid_period,
                                              scope, application_hint);
  struct token_response ret = _getTokenResponseFromRequest(request);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}

struct token_response getTokenResponseForGlobalDefaultConfig(
    time_t min_valid_period, const char* scope, const char* application_hint) {
  // TODO
}

char* getAccessToken(const char* accountname, time_t min_valid_period,
                     const char* scope) {
  START_APILOGLEVEL
  struct token_response response =
      getTokenResponse(accountname, min_valid_period, scope, NULL);
  secFree(response.issuer);
  END_APILOGLEVEL
  return response.token;
}

char* getAccessToken2(const char* accountname, time_t min_valid_period,
                      const char* scope, const char* application_hint) {
  START_APILOGLEVEL
  struct token_response response =
      getTokenResponse(accountname, min_valid_period, scope, application_hint);
  secFree(response.issuer);
  END_APILOGLEVEL
  return response.token;
}

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }

void secFreeTokenResponse(struct token_response token_response) {
  START_APILOGLEVEL
  secFree(token_response.token);
  secFree(token_response.issuer);
  END_APILOGLEVEL
}
