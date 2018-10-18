#include "api.h"
#include "ipc/communicator.h"
#include "ipc/ipc_values.h"
#include "json.h"
#include "oidc_error.h"
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

char* getAccountRequest() {
  START_APILOGLEVEL
  char* fmt = "{\"request\":\"%s\"}";
  char* ret = oidc_sprintf(fmt, REQUEST_VALUE_ACCOUNTLIST);
  END_APILOGLEVEL
  return ret;
}

char* getAccessTokenRequest(const char*   accountname,
                            unsigned long min_valid_period, const char* scope) {
  START_APILOGLEVEL
  char* fmt = strValid(scope) ? "{\"request\":\"%s\", \"account\":\"%s\", "
                                "\"min_valid_period\":%lu, \"scope\":\"%s\"}"
                              : "{\"request\":\"%s\", \"account\":\"%s\", "
                                "\"min_valid_period\":%lu}";
  char* ret = oidc_sprintf(fmt, REQUEST_VALUE_ACCESSTOKEN, accountname,
                           min_valid_period, scope);
  END_APILOGLEVEL
  return ret;
}

char* communicate(char* fmt, ...) {
  START_APILOGLEVEL
  va_list args;
  va_start(args, fmt);

  char* ret = ipc_vcommunicate(fmt, args);
  END_APILOGLEVEL
  return ret;
}

struct token_response getTokenResponse(const char*   accountname,
                                       unsigned long min_valid_period,
                                       const char*   scope) {
  START_APILOGLEVEL
  char* request  = getAccessTokenRequest(accountname, min_valid_period, scope);
  char* response = communicate(request);
  secFree(request);
  if (response == NULL) {
    END_APILOGLEVEL
    return (struct token_response){NULL, NULL};
  }
  struct key_value pairs[4];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "access_token";
  pairs[3].key = "issuer";
  if (getJSONValuesFromString(response, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    printError("Read malformed data. Please hand in bug report.\n");
    secFree(response);
    END_APILOGLEVEL
    return (struct token_response){NULL, NULL};
  }
  secFree(response);
  if (pairs[1].value) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    END_APILOGLEVEL
    return (struct token_response){NULL, NULL};
  } else {
    secFree(pairs[0].value);
    oidc_errno = OIDC_SUCCESS;
    END_APILOGLEVEL
    return (struct token_response){pairs[2].value, pairs[3].value};
  }
}

char* getAccessToken(const char* accountname, unsigned long min_valid_period,
                     const char* scope) {
  START_APILOGLEVEL
  struct token_response response =
      getTokenResponse(accountname, min_valid_period, scope);
  secFree(response.issuer);
  END_APILOGLEVEL
  return response.token;
}

char* getLoadedAccounts() {
  START_APILOGLEVEL
  char* request  = getAccountRequest();
  char* response = communicate(request);
  secFree(request);
  if (response == NULL) {
    END_APILOGLEVEL
    return NULL;
  }
  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "account_list";
  if (getJSONValuesFromString(response, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    printError("Read malformed data. Please hand in bug report.\n");
    secFree(response);
    END_APILOGLEVEL
    return NULL;
  }
  secFree(response);
  if (pairs[1].value) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    secFree(pairs[0].value);
    secFree(pairs[1].value);
    secFree(pairs[2].value);
    END_APILOGLEVEL
    return NULL;
  } else {
    secFree(pairs[0].value);
    oidc_errno = OIDC_SUCCESS;
    END_APILOGLEVEL
    return pairs[2].value;
  }
}

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }

void secFreeTokenResponse(struct token_response token_response) {
  START_APILOGLEVEL
  secFree(token_response.token);
  secFree(token_response.issuer);
  END_APILOGLEVEL
}
