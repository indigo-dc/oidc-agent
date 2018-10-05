#include "api.h"
#include "json.h"
#include "settings.h"
#include "oidc_error.h"
#include "ipc/ipc_values.h"
#include "ipc/communicator.h"

#include <stdlib.h>
#include <stdarg.h>

char* getAccountRequest() {
  char* fmt = "{\"request\":\"%s\"}";
  return oidc_sprintf(fmt, REQUEST_VALUE_ACCOUNTLIST);
}

char* getAccessTokenRequest(const char* accountname, unsigned long min_valid_period, const char* scope) {
  char* fmt = strValid(scope) ? 
    "{\"request\":\"%s\", \"account\":\"%s\", \"min_valid_period\":%lu, \"scope\":\"%s\"}" :
    "{\"request\":\"%s\", \"account\":\"%s\", \"min_valid_period\":%lu}";
  return oidc_sprintf(fmt, REQUEST_VALUE_ACCESSTOKEN, accountname, min_valid_period, scope);
}

char* communicate(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  return ipc_vcommunicate(fmt, args);
}

/** @fn char* getAccessToken(const char* accountname, unsigned long min_valid_period) 
 * @brief gets a valid access token for an account config
 * @param accountname the short name of the account config for which an access token
 * should be returned
 * @param min_valid_period the minium period of time the access token has to be valid
 * in seconds
 * @param scope a space delimited list of scope values for the to be issued
 * access token. NULL if default value for that account configuration should be
 * used.
 * @return a pointer to the access token. Has to be freed after usage. On
 * failure NULL is returned and oidc_errno is set.
 */
char* getAccessToken(const char* accountname, unsigned long min_valid_period, const char* scope) {
  char* request = getAccessTokenRequest(accountname, min_valid_period, scope);
  char* response = communicate(request);
  clearFreeString(request);
  if(response==NULL) {
    return NULL;
  }
  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "access_token";
  if(getJSONValues(response, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Read malformed data. Please hand in bug report.\n");
    clearFreeString(response);
    return NULL;
  }
  clearFreeString(response);
  if(pairs[1].value) { // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    clearFreeString(pairs[0].value);
    clearFreeString(pairs[1].value);
    clearFreeString(pairs[2].value);
    return NULL;
  } else {
    clearFreeString(pairs[0].value);
    oidc_errno = OIDC_SUCCESS;
    return pairs[2].value;
  }
}

/** @fn char* getLoadedAccount()
 * @brief gets a a list of currently loaded accounts
 * @return a pointer to the JSON Array String containing all the short names 
 * of the currently loaded accounts. Has to be freed after usage. 
 * On failure NULL is returned and oidc_errno is set.
 */
char* getLoadedAccounts() {
  char* request = getAccountRequest();
  char* response = communicate(request);
  clearFreeString(request);
  if(response==NULL) {
    return NULL;
  }
  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "account_list";
  if(getJSONValues(response, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Read malformed data. Please hand in bug report.\n");
    clearFreeString(response);
    return NULL;
  }
  clearFreeString(response);
  if(pairs[1].value) { // error
    oidc_errno = OIDC_EERROR;
    oidc_seterror(pairs[1].value);
    clearFreeString(pairs[0].value);
    clearFreeString(pairs[1].value);
    clearFreeString(pairs[2].value);
    return NULL;
  } else {
    clearFreeString(pairs[0].value);
    oidc_errno = OIDC_SUCCESS;
    return pairs[2].value;
  }
}

char* oidcagent_serror() {
  return oidc_serror();
}

void oidcagent_perror() {
  oidc_perror();
}
