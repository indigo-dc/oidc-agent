#include "api.h"

#include <stdarg.h>

#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "parse.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

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

char* communicate(unsigned char remote, const char* fmt, ...) {
  START_APILOGLEVEL
  if (fmt == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  va_list args;
  va_start(args, fmt);

  char* ret = ipc_vcryptCommunicate(remote, fmt, args);
  va_end(args);
  END_APILOGLEVEL
  return ret;
}

unsigned char _checkLocalResponseForRemote(struct token_response res) {
  if (res.token != NULL) {
    return LOCAL_COMM;
  }
  const char* err = oidc_serror();
  if (strequal(err, "No account configured with that short name") ||
      strstarts(err, "Could not connect to oidc-agent") ||
      strequal(err, "OIDC_SOCK env var not set")) {
    return REMOTE_COMM;
  }
  return LOCAL_COMM;
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

struct token_response _getTokenResponseFromRequest(unsigned char remote,
                                                   const char*   ipc_request) {
  char* response = communicate(remote, ipc_request);
  return parseForTokenResponse(response);
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
  char* request = getAccessTokenRequest(accountname, min_valid_period, scope,
                                        application_hint, audience);
  struct token_response ret = _getTokenResponseFromRequest(LOCAL_COMM, request);
  struct oidc_error_state* localError = saveErrorState();
  const unsigned char      remote     = _checkLocalResponseForRemote(ret);
  if (remote) {
    ret = _getTokenResponseFromRequest(remote, request);
    if (ret.token == NULL) {
      restoreErrorState(localError);
    }
  }
  secFreeErrorState(localError);
  secFree(request);
  END_APILOGLEVEL
  return ret;
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
  char* request = getAccessTokenRequestIssuer(
      issuer_url, min_valid_period, scope, application_hint, audience);
  struct token_response ret = _getTokenResponseFromRequest(LOCAL_COMM, request);
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

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }

void secFreeTokenResponse(struct token_response token_response) {
  START_APILOGLEVEL
  secFree(token_response.token);
  secFree(token_response.issuer);
  END_APILOGLEVEL
}
