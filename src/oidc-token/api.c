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

char* getAccessTokenRequest(const char*   accountname,
                            unsigned long min_valid_period, const char* scope,
                            const char* hint) {
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
  char* ret = jsonToString(json);
  secFreeJson(json);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", ret);
  END_APILOGLEVEL
  return ret;
}

char* communicate(char* fmt, ...) {
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

struct token_response getTokenResponse(const char*   accountname,
                                       unsigned long min_valid_period,
                                       const char*   scope,
                                       const char*   application_hint) {
  START_APILOGLEVEL
  char* request  = getAccessTokenRequest(accountname, min_valid_period, scope,
                                        application_hint);
  char* response = communicate(request);
  secFree(request);
  if (response == NULL) {
    END_APILOGLEVEL
    return (struct token_response){NULL, NULL, 0};
  }
  struct key_value pairs[5];
  pairs[0].key = IPC_KEY_STATUS;
  pairs[1].key = OIDC_KEY_ERROR;
  pairs[2].key = OIDC_KEY_ACCESSTOKEN;
  pairs[3].key = OIDC_KEY_ISSUER;
  pairs[4].key = AGENT_KEY_EXPIRESAT;
  if (getJSONValuesFromString(response, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    printError("Read malformed data. Please hand in bug report.\n");
    secFree(response);
    END_APILOGLEVEL
    return (struct token_response){NULL, NULL, 0};
  }
  secFree(response);
  if (pairs[1].value) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    END_APILOGLEVEL
    return (struct token_response){NULL, NULL, 0};
  } else {
    secFree(pairs[0].value);
    oidc_errno        = OIDC_SUCCESS;
    char*  end        = NULL;
    time_t expires_at = strtol(pairs[4].value, &end, 10);
    secFree(pairs[4].value);
    END_APILOGLEVEL
    return (struct token_response){pairs[2].value, pairs[3].value, expires_at};
  }
}

char* getAccessToken(const char* accountname, unsigned long min_valid_period,
                     const char* scope) {
  START_APILOGLEVEL
  struct token_response response =
      getTokenResponse(accountname, min_valid_period, scope, NULL);
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
