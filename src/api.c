#include "api.h"
#include "ipc/communicator.h"
#include "ipc/ipc_values.h"
#include "json.h"
#include "oidc_error.h"
#include "utils/printer.h"

#include <stdarg.h>
#include <stdlib.h>

char* getAccountRequest() {
  char* fmt = "{\"request\":\"%s\"}";
  return oidc_sprintf(fmt, REQUEST_VALUE_ACCOUNTLIST);
}

char* getAccessTokenRequest(const char*   accountname,
                            unsigned long min_valid_period, const char* scope) {
  char* fmt = strValid(scope) ? "{\"request\":\"%s\", \"account\":\"%s\", "
                                "\"min_valid_period\":%lu, \"scope\":\"%s\"}"
                              : "{\"request\":\"%s\", \"account\":\"%s\", "
                                "\"min_valid_period\":%lu}";
  return oidc_sprintf(fmt, REQUEST_VALUE_ACCESSTOKEN, accountname,
                      min_valid_period, scope);
}

char* communicate(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  return ipc_vcommunicate(fmt, args);
}

struct token_response getTokenResponse(const char*   accountname,
                                       unsigned long min_valid_period,
                                       const char*   scope) {
  char* request  = getAccessTokenRequest(accountname, min_valid_period, scope);
  char* response = communicate(request);
  secFree(request);
  if (response == NULL) {
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
    return (struct token_response){NULL, NULL};
  }
  secFree(response);
  if (pairs[1].value) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    return (struct token_response){NULL, NULL};
  } else {
    secFree(pairs[0].value);
    oidc_errno = OIDC_SUCCESS;
    return (struct token_response){pairs[2].value, pairs[3].value};
  }
}

char* getAccessToken(const char* accountname, unsigned long min_valid_period,
                     const char* scope) {
  struct token_response response =
      getTokenResponse(accountname, min_valid_period, scope);
  secFree(response.issuer);
  return response.token;
}

char* getLoadedAccounts() {
  char* request  = getAccountRequest();
  char* response = communicate(request);
  secFree(request);
  if (response == NULL) {
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
    return NULL;
  }
  secFree(response);
  if (pairs[1].value) {  // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    secFree(pairs[0].value);
    secFree(pairs[1].value);
    secFree(pairs[2].value);
    return NULL;
  } else {
    secFree(pairs[0].value);
    oidc_errno = OIDC_SUCCESS;
    return pairs[2].value;
  }
}

char* oidcagent_serror() { return oidc_serror(); }

void oidcagent_perror() { oidc_perror(); }

void secFreeTokenResponse(struct token_response token_response) {
  secFree(token_response.token);
  secFree(token_response.issuer);
}
