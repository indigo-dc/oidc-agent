#include "api.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "ipc/cryptCommunicator.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
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

char* communicate(const char* fmt, ...) {
  START_APILOGLEVEL
  if (fmt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  va_list args;
  va_start(args, fmt);

  char* ret = ipc_vcryptCommunicate(fmt, args);
  va_end(args);
  END_APILOGLEVEL
  return ret;
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
                                        application_hint, NULL);
  struct token_response ret = _getTokenResponseFromRequest(request);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}

struct token_response getTokenResponse3(const char* accountname,
                                        time_t      min_valid_period,
                                        const char* scope,
                                        const char* application_hint,
                                        const char* audience) {
  START_APILOGLEVEL
  char* request = getAccessTokenRequest(accountname, min_valid_period, scope,
                                        application_hint, audience);
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
                                              scope, application_hint, NULL);
  struct token_response ret = _getTokenResponseFromRequest(request);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}

struct token_response getTokenResponseForIssuer3(const char* issuer_url,
                                                 time_t      min_valid_period,
                                                 const char* scope,
                                                 const char* application_hint,
                                                 const char* audience) {
  START_APILOGLEVEL
  char* request = getAccessTokenRequestIssuer(
      issuer_url, min_valid_period, scope, application_hint, audience);
  struct token_response ret = _getTokenResponseFromRequest(request);
  secFree(request);
  END_APILOGLEVEL
  return ret;
}

char* getAccessToken(const char* accountname, time_t min_valid_period,
                     const char* scope) {
  return getAccessToken2(accountname, min_valid_period, scope, NULL);
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
  struct token_response response = getTokenResponseForIssuer(
      issuer_url, min_valid_period, scope, application_hint);
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

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }

void secFreeTokenResponse(struct token_response token_response) {
  START_APILOGLEVEL
  secFree(token_response.token);
  secFree(token_response.issuer);
  END_APILOGLEVEL
}
