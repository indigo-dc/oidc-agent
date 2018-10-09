#include "agent_handler.h"

#include "crypt.h"
#include "device_code.h"
#include "httpserver/httpserver.h"
#include "ipc/ipc.h"
#include "ipc/ipc_values.h"
#include "oidc_flows/access_token_handler.h"
#include "oidc_flows/code.h"
#include "oidc_flows/device.h"
#include "oidc_flows/openid_config.h"
#include "oidc_flows/registration.h"
#include "oidc_flows/revoke.h"

#include "../lib/list/src/list.h"

#include <string.h>
#include <strings.h>
#include <syslog.h>

void initAuthCodeFlow(struct oidc_account* account, int sock, char* info) {
  char state[25];
  randomFillHex(state, sizeof(state));
  char* uri = buildCodeFlowUri(account, state);
  if (uri == NULL) {
    ipc_writeOidcErrno(sock);
  } else {
    if (info) {
      ipc_write(sock, RESPONSE_STATUS_CODEURI_INFO, STATUS_ACCEPTED, uri, state,
                info);
    } else {
      ipc_write(sock, RESPONSE_STATUS_CODEURI, STATUS_ACCEPTED, uri, state);
    }
  }
  clearFreeString(uri);
}
void agent_handleGen(int sock, list_t* loaded_accounts, char* account_json,
                     const char* flow) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Gen request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if (!strValid(account_getTokenEndpoint(*account))) {
    ipc_writeOidcErrno(sock);
    clearFreeAccount(account);
    return;
  }

  int              success = 0;
  list_t*          flows   = parseFlow(flow);
  list_node_t*     current_flow;
  list_iterator_t* it = list_iterator_new(flows, LIST_HEAD);
  while ((current_flow = list_iterator_next(it))) {
    if (strcasecmp(current_flow->val, FLOW_VALUE_REFRESH) == 0) {
      if (getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL) !=
          NULL) {
        success = 1;
        break;
      } else if (flows->len == 1) {
        ipc_writeOidcErrno(sock);
        list_iterator_destroy(it);
        list_destroy(flows);
        clearFreeAccount(account);
        return;
      }
    } else if (strcasecmp(current_flow->val, FLOW_VALUE_PASSWORD) == 0) {
      if (getAccessTokenUsingPasswordFlow(account) == OIDC_SUCCESS) {
        success = 1;
        break;
      } else if (flows->len == 1) {
        ipc_writeOidcErrno(sock);
        list_iterator_destroy(it);
        list_destroy(flows);
        clearFreeAccount(account);
        return;
      }
    } else if (strcasecmp(current_flow->val, FLOW_VALUE_CODE) == 0 &&
               hasRedirectUris(*account)) {
      initAuthCodeFlow(account, sock, NULL);
      list_iterator_destroy(it);
      list_destroy(flows);
      clearFreeAccount(account);
      return;
    } else if (strcasecmp(current_flow->val, FLOW_VALUE_DEVICE) == 0) {
      struct oidc_device_code* dc = initDeviceFlow(account);
      if (dc == NULL) {
        ipc_writeOidcErrno(sock);
        list_iterator_destroy(it);
        list_destroy(flows);
        clearFreeAccount(account);
        return;
      }
      char* json = deviceCodeToJSON(*dc);
      ipc_write(sock, RESPONSE_ACCEPTED_DEVICE, json, account_json);
      clearFreeString(json);
      clearFreeDeviceCode(dc);
      list_iterator_destroy(it);
      list_destroy(flows);
      clearFreeAccount(account);
      return;
    } else {  // UNKNOWN FLOW
      ipc_write(sock, RESPONSE_ERROR, "Unknown flow %s", current_flow->val);
      list_iterator_destroy(it);
      list_destroy(flows);
      clearFreeAccount(account);
      return;
    }
  }

  list_iterator_destroy(it);
  list_destroy(flows);

  account_setUsername(account, NULL);
  account_setPassword(account, NULL);
  if (strValid(account_getRefreshToken(*account)) && success) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    clearFreeString(json);
    if (list_find(loaded_accounts, account)) {
      list_remove(loaded_accounts, list_find(loaded_accounts, account));
    }
    list_rpush(loaded_accounts, list_node_new(account));
  } else {
    ipc_write(sock, RESPONSE_ERROR,
              success ? "OIDP response does not contain a refresh token"
                      : "No flow was successfull.");
    clearFreeAccount(account);
  }
}

void agent_handleAdd(int sock, list_t* loaded_accounts, char* account_json) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Add request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if (NULL != list_find(loaded_accounts, account)) {
    clearFreeAccount(account);
    ipc_write(sock, RESPONSE_ERROR, "account already loaded");
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if (!strValid(account_getTokenEndpoint(*account))) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if (getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL) == NULL) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  list_rpush(loaded_accounts, list_node_new(account));
  ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleRm(int sock, list_t* loaded_accounts, char* account_json,
                    int revoke) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Remove request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if (NULL == list_find(loaded_accounts, account)) {
    clearFreeAccount(account);
    ipc_write(sock, RESPONSE_ERROR,
              revoke ? "Could not revoke token: account not loaded"
                     : "account not loaded");
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if (revoke && (revokeToken(account) != OIDC_SUCCESS)) {
    clearFreeAccount(account);
    char* error = oidc_sprintf("Could not revoke token: %s", oidc_serror());
    ipc_write(sock, RESPONSE_ERROR, error);
    clearFreeString(error);
    return;
  }
  if (list_find(loaded_accounts, account)) {
    list_remove(loaded_accounts, list_find(loaded_accounts, account));
  }
  clearFreeAccount(account);
  ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleToken(int sock, list_t* loaded_accounts, char* short_name,
                       char* min_valid_period_str, const char* scope) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Token request");
  if (short_name == NULL) {
    ipc_write(sock, RESPONSE_ERROR,
              "Bad request. Required field 'account_name' not present.");
    return;
  }
  struct oidc_account key = {.shortname = short_name};
  time_t              min_valid_period =
      min_valid_period_str != NULL ? atoi(min_valid_period_str) : 0;
  struct list_node* account_node = list_find(loaded_accounts, &key);
  if (account_node == NULL) {
    ipc_write(sock, RESPONSE_ERROR, "Account not loaded.");
    return;
  }
  char* access_token = getAccessTokenUsingRefreshFlow(account_node->val,
                                                      min_valid_period, scope);
  if (access_token == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  ipc_write(sock, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token,
            account_getIssuerUrl(*(struct oidc_account*)(account_node->val)));
  if (strValid(scope)) {
    clearFreeString(access_token);
  }
}

void agent_handleList(int sock, list_t* loaded_accounts) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle list request");
  char* accountList = getAccountNameList(loaded_accounts);
  ipc_write(sock, RESPONSE_STATUS_ACCOUNT, STATUS_SUCCESS,
            oidc_errno == OIDC_EARGNULL ? "[]" : accountList);
  clearFreeString(accountList);
}

void agent_handleRegister(int sock, list_t* loaded_accounts, char* account_json,
                          const char* access_token) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Register request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if (NULL != list_find(loaded_accounts, account)) {
    clearFreeAccount(account);
    ipc_write(sock, RESPONSE_ERROR,
              "An account with this shortname is already loaded. I will not "
              "register a new one.");
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  char* res = dynamicRegistration(account, 1, access_token);
  if (res == NULL) {
    ipc_writeOidcErrno(sock);
  } else {
    if (!isJSONObject(res)) {
      char* escaped = escapeCharInStr(res, '"');
      ipc_write(sock, RESPONSE_ERROR_INFO,
                "Received no JSON formatted response.", escaped);
      clearFreeString(escaped);
    } else {
      if (json_hasKey(res, "error")) {  // first failed
        char* res2 = dynamicRegistration(account, 0, access_token);
        if (res2 == NULL) {  // second failed complety
          ipc_writeOidcErrno(sock);
        } else {
          if (json_hasKey(res2, "error")) {  // first and second failed
            char* error = getJSONValue(res, "error_description");
            if (error == NULL) {
              error = getJSONValue(res, "error");
            }
            ipc_write(sock, RESPONSE_ERROR, error);
            clearFreeString(error);
          } else {  // first failed, second successful
            ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res2);
          }
        }
        clearFreeString(res2);
      } else {  // first was successfull
        ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res);
      }
    }
  }
  clearFreeString(res);
  clearFreeAccount(account);
}

void agent_handleCodeExchange(int sock, list_t* loaded_accounts,
                              char* account_json, char* code,
                              char* redirect_uri, char* state) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle codeExchange request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if (getAccessTokenUsingAuthCodeFlow(account, code, redirect_uri) !=
      OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    return;
  }
  if (strValid(account_getRefreshToken(*account))) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    clearFreeString(json);
    account_setUsedState(account, oidc_sprintf("%s", state));
    if (list_find(loaded_accounts, account)) {
      list_remove(loaded_accounts, list_find(loaded_accounts, account));
    }
    list_rpush(loaded_accounts, list_node_new(account));
  } else {
    ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");
    clearFreeAccount(account);
  }
}

void agent_handleDeviceLookup(int sock, list_t* loaded_accounts,
                              char* account_json, char* device_json) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle deviceLookup request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrno(sock);
    return;
  }
  struct oidc_device_code* dc = getDeviceCodeFromJSON(device_json);
  if (dc == NULL) {
    ipc_writeOidcErrno(sock);
    clearFreeAccount(account);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    clearFreeAccount(account);
    ipc_writeOidcErrno(sock);
    clearFreeDeviceCode(dc);
    return;
  }
  if (getAccessTokenUsingDeviceFlow(account, oidc_device_getDeviceCode(*dc)) !=
      OIDC_SUCCESS) {
    clearFreeAccount(account);
    clearFreeDeviceCode(dc);
    ipc_writeOidcErrno(sock);
    return;
  }
  clearFreeDeviceCode(dc);
  if (strValid(account_getRefreshToken(*account))) {
    char* json = accountToJSON(*account);
    ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    clearFreeString(json);
    if (list_find(loaded_accounts, account)) {
      list_remove(loaded_accounts, list_find(loaded_accounts, account));
    }
    list_rpush(loaded_accounts, list_node_new(account));
  } else {
    ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");
    clearFreeAccount(account);
  }
}

void agent_handleStateLookUp(int sock, list_t* loaded_accounts, char* state) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle codeLookUp request");
  struct oidc_account key      = {.usedState = state};
  void*               oldMatch = loaded_accounts->match;
  loaded_accounts->match       = (int (*)(void*, void*)) & account_matchByState;
  struct list_node* account_node = list_find(loaded_accounts, &key);
  loaded_accounts->match         = oldMatch;
  if (account_node == NULL) {
    char* info =
        oidc_sprintf("No loaded account info found for state=%s", state);
    ipc_write(sock, RESPONSE_STATUS_INFO, STATUS_NOTFOUND, info);
    clearFreeString(info);
    return;
  }
  struct oidc_account* account = account_node->val;
  account_setUsedState(account, NULL);
  char* config = accountToJSON(*account);
  ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, config);
  clearFreeString(config);
  termHttpServer(state);
}

void agent_handleTermHttp(int sock, char* state) {
  termHttpServer(state);
  ipc_write(sock, RESPONSE_SUCCESS);
}
