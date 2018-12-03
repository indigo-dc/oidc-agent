#include "agent_handler.h"

#include "agent_state.h"
#include "httpserver/startHttpserver.h"
#include "httpserver/termHttpserver.h"
#include "ipc/cryptIpc.h"
#include "ipc/ipc.h"
#include "ipc/ipc_values.h"
#include "list/list.h"
#include "oidc/device_code.h"
#include "oidc/flows/access_token_handler.h"
#include "oidc/flows/code.h"
#include "oidc/flows/device.h"
#include "oidc/flows/openid_config.h"
#include "oidc/flows/registration.h"
#include "oidc/flows/revoke.h"
#include "utils/crypt.h"
#include "utils/json.h"
#include "utils/listUtils.h"

#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>

void initAuthCodeFlow(const struct oidc_account* account, int sock,
                      const char* info) {
  char state[25];
  randomFillBase64UrlSafe(state, sizeof(state));
  char* uri = buildCodeFlowUri(account, state);
  if (uri == NULL) {
    server_ipc_writeOidcErrno(sock);
  } else {
    if (info) {
      server_ipc_write(sock, RESPONSE_STATUS_CODEURI_INFO, STATUS_ACCEPTED, uri,
                       state, info);
    } else {
      server_ipc_write(sock, RESPONSE_STATUS_CODEURI, STATUS_ACCEPTED, uri,
                       state);
    }
  }
  secFree(uri);
}
void agent_handleGen(int sock, list_t* loaded_accounts,
                     const char* account_json, const char* flow) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Gen request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (!strValid(account_getTokenEndpoint(*account))) {
    server_ipc_writeOidcErrno(sock);
    secFreeAccount(account);
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
        server_ipc_writeOidcErrno(sock);
        list_iterator_destroy(it);
        list_destroy(flows);
        secFreeAccount(account);
        return;
      }
    } else if (strcasecmp(current_flow->val, FLOW_VALUE_PASSWORD) == 0) {
      if (getAccessTokenUsingPasswordFlow(account) == OIDC_SUCCESS) {
        success = 1;
        break;
      } else if (flows->len == 1) {
        server_ipc_writeOidcErrno(sock);
        list_iterator_destroy(it);
        list_destroy(flows);
        secFreeAccount(account);
        return;
      }
    } else if (strcasecmp(current_flow->val, FLOW_VALUE_CODE) == 0 &&
               hasRedirectUris(*account)) {
      initAuthCodeFlow(account, sock, NULL);
      list_iterator_destroy(it);
      list_destroy(flows);
      secFreeAccount(account);
      return;
    } else if (strcasecmp(current_flow->val, FLOW_VALUE_DEVICE) == 0) {
      struct oidc_device_code* dc = initDeviceFlow(account);
      if (dc == NULL) {
        server_ipc_writeOidcErrno(sock);
        list_iterator_destroy(it);
        list_destroy(flows);
        secFreeAccount(account);
        return;
      }
      char* json = deviceCodeToJSON(*dc);
      server_ipc_write(sock, RESPONSE_ACCEPTED_DEVICE, json, account_json);
      secFree(json);
      secFreeDeviceCode(dc);
      list_iterator_destroy(it);
      list_destroy(flows);
      secFreeAccount(account);
      return;
    } else {  // UNKNOWN FLOW
      server_ipc_write(sock, RESPONSE_ERROR, "Unknown flow %s",
                       current_flow->val);
      list_iterator_destroy(it);
      list_destroy(flows);
      secFreeAccount(account);
      return;
    }
  }

  list_iterator_destroy(it);
  list_destroy(flows);

  account_setUsername(account, NULL);
  account_setPassword(account, NULL);
  if (account_refreshTokenIsValid(*account) && success) {
    char* json = accountToJSONString(*account);
    server_ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    addAccountToList(loaded_accounts, account);
  } else {
    server_ipc_write(sock, RESPONSE_ERROR,
                     success ? "OIDP response does not contain a refresh token"
                             : "No flow was successfull.");
    secFreeAccount(account);
  }
}

void agent_handleAdd(int sock, list_t* loaded_accounts,
                     const char* account_json, const char* timeout_str) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Add request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  size_t timeout = 0;
  if (strValid(timeout_str)) {
    timeout = atol(timeout_str);
  } else {
    timeout = agent_state.defaultTimeout;
  }
  account_setDeath(account, timeout ? time(NULL) + timeout : 0);
  struct oidc_account* found = NULL;
  if ((found = getAccountFromList(loaded_accounts, account)) != NULL) {
    if (account_getDeath(*found) != account_getDeath(*account)) {
      account_setDeath(found, account_getDeath(*account));
      char* msg = NULL;
      if (timeout == 0) {
        msg = oidc_sprintf("account already loaded. Lifetime set to infinity.");
      } else {
        msg = oidc_sprintf(
            "account already loaded. Lifetime set to %lu seconds.", timeout);
      }
      server_ipc_write(sock, RESPONSE_SUCCESS_INFO, msg);
      secFree(msg);
    } else {
      server_ipc_write(sock, RESPONSE_SUCCESS_INFO, "account already loaded.");
    }
    addAccountToList(loaded_accounts, found);  // reencrypting sensitive data
    secFreeAccount(account);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (!strValid(account_getTokenEndpoint(*account))) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL) == NULL) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  addAccountToList(loaded_accounts, account);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Loaded Account. Used timeout of %lu",
         timeout);
  if (timeout > 0) {
    char* msg = oidc_sprintf("Lifetime set to %lu seconds", timeout);
    server_ipc_write(sock, RESPONSE_SUCCESS_INFO, msg);
    secFree(msg);
  } else {
    server_ipc_write(sock, RESPONSE_STATUS_SUCCESS);
  }
}

void agent_handleDelete(int sock, list_t* loaded_accounts,
                        const char* account_json) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Delete request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  list_node_t* found_node = NULL;
  if ((found_node = findInList(loaded_accounts, account)) == NULL) {
    secFreeAccount(account);
    server_ipc_write(sock, RESPONSE_ERROR,
                     "Could not revoke token: account not loaded");
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (revokeToken(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    char* error = oidc_sprintf("Could not revoke token: %s", oidc_serror());
    server_ipc_write(sock, RESPONSE_ERROR, error);
    secFree(error);
    return;
  }
  list_remove(loaded_accounts, found_node);
  secFreeAccount(account);
  server_ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleRm(int sock, list_t* loaded_accounts, char* account_name) {
  if (account_name == NULL) {
    server_ipc_write(
        sock, RESPONSE_BADREQUEST,
        "Have to provide shortname of the account config that should be "
        "removed.");
    return;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Remove request for config '%s'",
         account_name);
  struct oidc_account key   = {.shortname = account_name};
  list_node_t*        found = NULL;
  if ((found = findInList(loaded_accounts, &key)) == NULL) {
    server_ipc_write(sock, RESPONSE_ERROR, "account not loaded");
    return;
  }
  list_remove(loaded_accounts, found);
  server_ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleRemoveAll(int sock, list_t** loaded_accounts) {
  list_t* empty = list_new();
  empty->free   = (*loaded_accounts)->free;
  empty->match  = (*loaded_accounts)->match;
  list_destroy(*loaded_accounts);
  *loaded_accounts = empty;
  server_ipc_write(sock, RESPONSE_STATUS_SUCCESS);
}

void agent_handleToken(int sock, list_t* loaded_accounts, char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char* application_hint) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Token request from %s",
         application_hint);
  if (short_name == NULL) {
    server_ipc_write(sock, RESPONSE_ERROR,
                     "Bad request. Required field 'account_name' not present.");
    return;
  }
  struct oidc_account key = {.shortname = short_name};
  time_t              min_valid_period =
      min_valid_period_str != NULL ? atoi(min_valid_period_str) : 0;
  struct oidc_account* account = getAccountFromList(loaded_accounts, &key);
  if (account == NULL) {
    server_ipc_write(sock, RESPONSE_ERROR, "Account not loaded.");
    return;
  }
  char* access_token =
      getAccessTokenUsingRefreshFlow(account, min_valid_period, scope);
  addAccountToList(loaded_accounts, account);  // reencrypting
  if (access_token == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  server_ipc_write(sock, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token,
                   account_getIssuerUrl(*account),
                   account_getTokenExpiresAt(*account));
  if (strValid(scope)) {
    secFree(access_token);
  }
}

/**
 * Removed in version 2.0.0
 */
// void agent_handleList(int sock, list_t* loaded_accounts) {
//   syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle list request");
//   char* accountList = getAccountNameList(loaded_accounts);
//   server_ipc_write(sock, RESPONSE_STATUS_ACCOUNT, STATUS_SUCCESS,
//             oidc_errno == OIDC_EARGNULL ? "[]" : accountList);
//   secFree(accountList);
// }

void agent_handleRegister(int sock, list_t* loaded_accounts,
                          const char* account_json, const char* flows_json_str,
                          const char* access_token) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Register request for flows: '%s'",
         flows_json_str);
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (NULL != findInList(loaded_accounts, account)) {
    secFreeAccount(account);
    server_ipc_write(
        sock, RESPONSE_ERROR,
        "An account with this shortname is already loaded. I will not "
        "register a new one.");
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  list_t* flows = JSONArrayStringToList(flows_json_str);
  if (flows == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  char* res = dynamicRegistration(account, flows, access_token);
  list_destroy(flows);
  if (res == NULL) {
    server_ipc_writeOidcErrno(sock);
  } else {
    if (!isJSONObject(res)) {
      char* escaped = escapeCharInStr(res, '"');
      server_ipc_write(sock, RESPONSE_ERROR_INFO,
                       "Received no JSON formatted response.", escaped);
      secFree(escaped);
    } else {
      cJSON* json_res1 = stringToJson(res);
      if (jsonHasKey(json_res1, "error")) {  // first failed
        list_removeIfFound(flows, list_find(flows, "password"));
        char* res2 = dynamicRegistration(account, flows, access_token);
        if (res2 == NULL) {  // second failed complety
          server_ipc_writeOidcErrno(sock);
        } else {
          if (jsonStringHasKey(res2, "error")) {  // first and second failed
            char* error = getJSONValue(json_res1, "error_description");
            if (error == NULL) {
              error = getJSONValue(json_res1, "error");
            }
            server_ipc_write(sock, RESPONSE_ERROR, error);
            secFree(error);
          } else {  // first failed, second successful
            server_ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res2);
          }
        }
        secFree(res2);
      } else {  // first was successfull
        server_ipc_write(sock, RESPONSE_SUCCESS_CLIENT, res);
      }
      secFreeJson(json_res1);
    }
  }
  secFree(res);
  secFreeAccount(account);
}

void agent_handleCodeExchange(int sock, list_t* loaded_accounts,
                              const char* account_json, const char* code,
                              const char* redirect_uri, const char* state) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle codeExchange request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (getAccessTokenUsingAuthCodeFlow(account, code, redirect_uri) !=
      OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  if (account_refreshTokenIsValid(*account)) {
    char* json = accountToJSONString(*account);
    server_ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    account_setUsedState(account, oidc_sprintf("%s", state));
    addAccountToList(loaded_accounts, account);
  } else {
    server_ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");
    secFreeAccount(account);
  }
}

void agent_handleDeviceLookup(int sock, list_t* loaded_accounts,
                              const char* account_json,
                              const char* device_json) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle deviceLookup request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    server_ipc_writeOidcErrno(sock);
    return;
  }
  struct oidc_device_code* dc = getDeviceCodeFromJSON(device_json);
  if (dc == NULL) {
    server_ipc_writeOidcErrno(sock);
    secFreeAccount(account);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    server_ipc_writeOidcErrno(sock);
    secFreeDeviceCode(dc);
    return;
  }
  if (getAccessTokenUsingDeviceFlow(account, oidc_device_getDeviceCode(*dc)) !=
      OIDC_SUCCESS) {
    secFreeAccount(account);
    secFreeDeviceCode(dc);
    server_ipc_writeOidcErrno(sock);
    return;
  }
  secFreeDeviceCode(dc);
  if (account_refreshTokenIsValid(*account)) {
    char* json = accountToJSONString(*account);
    server_ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    addAccountToList(loaded_accounts, account);
  } else {
    server_ipc_write(sock, RESPONSE_ERROR, "Could not get a refresh token");
    secFreeAccount(account);
  }
}

void agent_handleStateLookUp(int sock, list_t* loaded_accounts, char* state) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle codeLookUp request");
  struct oidc_account key      = {.usedState = state};
  void*               oldMatch = loaded_accounts->match;
  loaded_accounts->match       = (int (*)(void*, void*)) & account_matchByState;
  struct oidc_account* account = getAccountFromList(loaded_accounts, &key);
  loaded_accounts->match       = oldMatch;
  if (account == NULL) {
    char* info =
        oidc_sprintf("No loaded account info found for state=%s", state);
    server_ipc_write(sock, RESPONSE_STATUS_INFO, STATUS_NOTFOUND, info);
    secFree(info);
    return;
  }
  account_setUsedState(account, NULL);
  char* config = accountToJSONString(*account);
  server_ipc_write(sock, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, config);
  secFree(config);
  addAccountToList(loaded_accounts, account);  // reencrypting
  termHttpServer(state);
}

void agent_handleTermHttp(int sock, const char* state) {
  termHttpServer(state);
  server_ipc_write(sock, RESPONSE_SUCCESS);
}

void agent_handleLock(int sock, const char* password, list_t* loaded_accounts,
                      int _lock) {
  if (_lock) {
    if (lock(loaded_accounts, password) == OIDC_SUCCESS) {
      server_ipc_write(sock, RESPONSE_SUCCESS_INFO, "Agent locked");
      return;
    }
  } else {
    if (unlock(loaded_accounts, password) == OIDC_SUCCESS) {
      server_ipc_write(sock, RESPONSE_SUCCESS_INFO, "Agent unlocked");
      return;
    }
  }
  server_ipc_writeOidcErrno(sock);
}
