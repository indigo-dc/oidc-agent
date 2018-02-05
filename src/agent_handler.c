#include "agent_handler.h"
#include "ipc.h"
#include "oidc.h"
#include "crypt.h"
#include "httpserver.h"
#include "flow_handler.h"

#include "../lib/list/src/list.h"

#include <syslog.h>
#include <string.h>
#include <strings.h>

void initAuthCodeFlow(struct oidc_account* account, int sock, char* info) {
  char state[25];
  randomFillHex(state, sizeof(state));
  char* uri = buildCodeFlowUri(account, state);
  if(uri == NULL) {
    ipc_writeOidcErrno(sock);
  } else {
    if(info) {
      ipc_write(sock, RESPONSE_STATUS_CODEURI_INFO, STATUS_ACCEPTED, uri, state, info);
    } else {
      ipc_write(sock, RESPONSE_STATUS_CODEURI, STATUS_ACCEPTED, uri, state);
    }
  }
  clearFreeString(uri);
}
void agent_handleGen(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, const char* flow) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Gen request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if(!isValid(account_getTokenEndpoint(*account))) {
    ipc_writeOidcErrno(sock);
    freeAccount(account);
    return;
  }

  int success = 0;
  list_t* flows = parseFlow(flow); 
  list_node_t* current_flow;
  list_iterator_t *it = list_iterator_new(flows, LIST_HEAD);
  while ((current_flow = list_iterator_next(it))) {
    if(strcasecmp(current_flow->val, FLOW_VALUE_REFRESH)==0) {
      if(getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL)!=NULL) {
        success = 1;
        break;
      }
    } else if(strcasecmp(current_flow->val, FLOW_VALUE_PASSWORD)==0) {
      if(getAccessTokenUsingPasswordFlow(account)==OIDC_SUCCESS) {
        success = 1;
        break;
      }
    } else if(strcasecmp(current_flow->val, FLOW_VALUE_CODE) == 0 && hasRedirectUris(*account)) {
      initAuthCodeFlow(account, sock, NULL);
      list_iterator_destroy(it);
      list_destroy(flows);
      freeAccount(account);
      return;
    } else { //UNKNOWN FLOW
      ipc_write(sock, RESPONSE_ERROR, "Unknown flow %s", current_flow->val);   
      list_iterator_destroy(it);
      list_destroy(flows);
      freeAccount(account);
      return;
    }
  }

  list_iterator_destroy(it);
  list_destroy(flows);

  account_setUsername(account, NULL);
  account_setPassword(account, NULL);
  if(isValid(account_getRefreshToken(*account))) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    clearFreeString(json);
    *loaded_p = removeAccount(*loaded_p, loaded_p_count, *account);
    *loaded_p = addAccount(*loaded_p, loaded_p_count, *account);
    clearFree(account, sizeof(*account));
  } else {
    ipc_write(sock, RESPONSE_ERROR, success ? "OIDP response does not contain a refresh token" : "No flow was successfull.");   
    freeAccount(account);
  }
} 

void agent_handleAdd(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Add request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(NULL!=findAccountByName(*loaded_p, *loaded_p_count, *account)) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, "account already loaded");
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if(!isValid(account_getTokenEndpoint(*account))) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL)==NULL) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  *loaded_p = addAccount(*loaded_p, loaded_p_count, *account);
  clearFree(account, sizeof(*account));
  ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleRm(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, int revoke) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Remove request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(NULL==findAccountByName(*loaded_p, *loaded_p_count, *account)) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, revoke ? "Could not revoke token: account not loaded" : "account not loaded");
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if(revoke && (revokeToken(account)!=OIDC_SUCCESS)) {
    freeAccount(account);
    char* error = oidc_sprintf("Could not revoke token: %s", oidc_serror());
    ipc_write(sock, RESPONSE_ERROR, error);
    clearFreeString(error);
    return;
  }
  *loaded_p = removeAccount(*loaded_p, loaded_p_count, *account);
  freeAccount(account);
  ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleToken(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* short_name, char* min_valid_period_str, const char* scope) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Token request");
  if(short_name==NULL) {
    ipc_write(sock, RESPONSE_ERROR, "Bad request. Required field 'account_name' not present.");
    return;
  }
  struct oidc_account key = { .name = short_name };
  time_t min_valid_period = min_valid_period_str!=NULL ? atoi(min_valid_period_str) : 0;
  struct oidc_account* account = findAccountByName(loaded_p, loaded_p_count, key);
  if(account==NULL) {
    ipc_write(sock, RESPONSE_ERROR, "Account not loaded.");
    return;
  }
  char* access_token = getAccessTokenUsingRefreshFlow(account, min_valid_period, scope);
  if(access_token==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  ipc_write(sock, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token);
  if(isValid(scope)) {
    clearFreeString(access_token);
  }
}

void agent_handleList(int sock, struct oidc_account* loaded_p, size_t loaded_p_count) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle list request");
  char* accountList = getAccountNameList(loaded_p, loaded_p_count);
  ipc_write(sock, RESPONSE_STATUS_ACCOUNT, STATUS_SUCCESS, oidc_errno==OIDC_EARGNULL ? "[]" : accountList);
  clearFreeString(accountList);
}

void agent_handleRegister(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* account_json, const char* access_token) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle Register request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(NULL!=findAccountByName(loaded_p, loaded_p_count, *account)) {
    freeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, "A account with this shortname is already loaded. I will not register a new one.");
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  char* res = dynamicRegistration(account, 1, access_token);
  if(res==NULL) {
    ipc_writeOidcErrno(sock);
  } else {
    if(!isJSONObject(res)) {
      char* escaped = escapeCharInStr(res, '"');
      ipc_write(sock, RESPONSE_ERROR_INFO, "Received no JSON formatted response.", escaped);
      clearFreeString(escaped);
    } else {
    if(json_hasKey(res, "error")) { // first failed
      char* res2 = dynamicRegistration(account, 0, access_token);
      if(res2==NULL) { //second failed complety
        ipc_writeOidcErrno(sock);
      } else {
        if(json_hasKey(res2, "error")) { // first and second failed
          char* error = getJSONValue(res, "error_description");
          if(error==NULL) {
            error = getJSONValue(res, "error");
          }
          ipc_write(sock, RESPONSE_ERROR, error); 
          clearFreeString(error);
        } else { // first failed, second successful
          ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res2);
        }
      }
      clearFreeString(res2);
    } else { // first was successfull
      ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res);
    }
  }
  }
  clearFreeString(res);
  freeAccount(account);
}

void agent_handleCodeExchange(int sock, struct oidc_account** loaded_p, size_t* loaded_p_count, char* account_json, char* code, char* redirect_uri, char* state) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle codeExchange request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if(account==NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if(getIssuerConfig(account)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if(getAccessTokenUsingAuthCodeFlow(account, code, redirect_uri)!=OIDC_SUCCESS) {
    freeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if(isValid(account_getRefreshToken(*account))) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    clearFreeString(json);
    account_setUsedState(account, oidc_sprintf("%s", state));
    *loaded_p = removeAccount(*loaded_p, loaded_p_count, *account);
    *loaded_p = addAccount(*loaded_p, loaded_p_count, *account);
    clearFree(account, sizeof(*account));
  } else {
    ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");   
    freeAccount(account);
  }
}

void agent_handleStateLookUp(int sock, struct oidc_account* loaded_p, size_t loaded_p_count, char* state) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Handle codeLookUp request");
  struct oidc_account key = { .usedState = state };
  struct oidc_account* account = findAccountByState(loaded_p, loaded_p_count, key);
  if(account==NULL) {
    char* info = oidc_sprintf("No loaded account info found for state=%s", state);
    ipc_write(sock, RESPONSE_STATUS_INFO, STATUS_NOTFOUND, info);
    clearFreeString(info);
    return;
  }
  account_setUsedState(account, NULL);
  char* config = accountToJSON(*account);
  ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, config);
  clearFreeString(config);
  termHttpServer(state);
}
