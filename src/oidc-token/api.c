#include "api.h"

#include <stdarg.h>

#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "parse.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

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

unsigned char _checkLocalResponseForRemote(struct agent_response res) {
  if (res.type == AGENT_RESPONSE_TYPE_TOKEN &&
      res.token_response.token != NULL) {
    return LOCAL_COMM;
  }
  const char* err =
      res.type == AGENT_RESPONSE_TYPE_ERROR && res.error_response.error != NULL
          ? res.error_response.error
          : oidc_serror();
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

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }

void oidcagent_printErrorResponse(struct agent_error_response err) {
  if (err.error) {
    printError("Error: %s\n", err.error);
  }
  if (err.help) {
    printImportant("%s", err.help);
  }
}

void secFreeTokenResponse(struct token_response token_response) {
  START_APILOGLEVEL
  secFree(token_response.token);
  secFree(token_response.issuer);
  END_APILOGLEVEL
}

void secFreeErrorResponse(struct agent_error_response error_response) {
  START_APILOGLEVEL
  secFree(error_response.error);
  secFree(error_response.help);
  END_APILOGLEVEL
}

void secFreeAgentResponse(struct agent_response agent_response) {
  START_APILOGLEVEL
  switch (agent_response.type) {
    case AGENT_RESPONSE_TYPE_ERROR:
      secFreeErrorResponse(agent_response.error_response);
      break;
    case AGENT_RESPONSE_TYPE_TOKEN:
      secFreeTokenResponse(agent_response.token_response);
      break;
  }
  END_APILOGLEVEL
}
