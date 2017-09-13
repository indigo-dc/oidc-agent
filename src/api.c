#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "api.h"
#include "ipc.h"
#include "json.h"
#include "oidc_utilities.h"
#include "oidc_error.h"


char* getAccountRequest() {
  char* fmt = "{\"request\":\"account_list\"}";
  return fmt;
}

char* getAccessTokenRequest(const char* accountname, unsigned long min_valid_period) {
  char* fmt = "{\"request\":\"access_token\", \"account\":\"%s\", \"min_valid_period\":%lu}";
  char* request = calloc(sizeof(char), snprintf(NULL, 0, fmt, accountname, min_valid_period)+1);
  sprintf(request, fmt, accountname, min_valid_period);
  return request;
}

char* communicate(char* json_request) {
  static struct connection con;
  if(ipc_init(&con, OIDC_SOCK_ENV_NAME, 0)!=OIDC_SUCCESS) { 
    return NULL; 
  }
  if(ipc_connect(con)<0) {
    return NULL;
  }
  ipc_write(*(con.sock), json_request);
  char* response = ipc_read(*(con.sock));
  ipc_close(&con);
  if(NULL==response) {
    fprintf(stderr, "An unexpected error occured. It seems that oidc-agent has stopped.\n%s\n", oidc_perror());
    exit(EXIT_FAILURE);
  }
  return response;
}

/** @fn char* getAccessToken(const char* accountname, unsigned long min_valid_period) 
 * @brief gets an valid access token for a account config
 * @param accountname the short name of the account config for which an access token
 * should be returned
 * @param min_valid_period the minium period of time the access token has to be valid
 * in seconds
 * @return a pointer to the access token. Has to be freed after usage. On
 * failure NULL is returned and oidc_errno is set.
 */
char* getAccessToken(const char* accountname, unsigned long min_valid_period) {
  char* request = getAccessTokenRequest(accountname, min_valid_period);
  char* response = communicate(request);
  if(response==NULL) {
    return NULL;
  }
  clearFreeString(request);
  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "access_token";
  if(getJSONValues(response, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    fprintf(stderr, "Read malformed data. Please hand in bug report.\n");
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
  if(response==NULL) {
    return NULL;
  }
  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "account_list";
  if(getJSONValues(response, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    fprintf(stderr, "Read malformed data. Please hand in bug report.\n");
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


