#include "oidcd_handler.h"

#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "list/list.h"
#include "oidc-agent/agent_state.h"
#include "oidc-agent/httpserver/startHttpserver.h"
#include "oidc-agent/httpserver/termHttpserver.h"
#include "oidc-agent/oidc/device_code.h"
#include "oidc-agent/oidc/flows/access_token_handler.h"
#include "oidc-agent/oidc/flows/code.h"
#include "oidc-agent/oidc/flows/device.h"
#include "oidc-agent/oidc/flows/openid_config.h"
#include "oidc-agent/oidc/flows/registration.h"
#include "oidc-agent/oidc/flows/revoke.h"
#include "oidc-agent/oidcd/codeExchangeEntry.h"
#include "oidc-agent/oidcd/parse_internal.h"
#include "utils/accountUtils.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/db/account_db.h"
#include "utils/db/codeVerifier_db.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/uriUtils.h"

#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>

void initAuthCodeFlow(struct oidc_account* account, struct ipcPipe pipes,
                      const char* info, const char* prioritizeCustom_str,
                      const struct arguments* arguments) {
  if (arguments->no_webserver || strToInt(prioritizeCustom_str)) {
    account_setNoWebServer(account);
  }
  size_t state_len       = 24;
  size_t socket_path_len = oidc_strlen(getServerSocketPath());
  char*  socket_path_base64 =
      toBase64UrlSafe(getServerSocketPath(), socket_path_len);
  // syslog(LOG_AUTHPRIV | LOG_DEBUG, "Base64 socket path is '%s'",
  //        socket_path_base64);
  char random[state_len + 1];
  randomFillBase64UrlSafe(random, state_len);
  random[state_len] = '\0';
  char* state =
      oidc_sprintf("%s:%lu:%s", random, socket_path_len, socket_path_base64);
  char** state_ptr = &state;
  secFree(socket_path_base64);

  char* code_verifier = secAlloc(CODE_VERIFIER_LEN + 1);
  randomFillBase64UrlSafe(code_verifier, CODE_VERIFIER_LEN);

  char* uri = buildCodeFlowUri(account, state_ptr, code_verifier);
  if (uri == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFree(code_verifier);
    secFree(*state_ptr);
    secFreeAccount(account);
    return;
  }
  // syslog(LOG_AUTHPRIV | LOG_DEBUG, "code_verifier for state '%s' is '%s'",
  //        state, code_verifier);
  codeVerifierDB_addValue(
      createCodeExchangeEntry(*state_ptr, account, code_verifier));
  if (info) {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CODEURI_INFO, STATUS_ACCEPTED, uri,
                    *state_ptr, info);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CODEURI, STATUS_ACCEPTED, uri,
                    *state_ptr);
  }
  secFree(uri);
}

void oidcd_handleGen(struct ipcPipe pipes, const char* account_json,
                     const char* flow, const char* prioritizeCustom_str,
                     const struct arguments* arguments) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Gen request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (!strValid(account_getTokenEndpoint(account))) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeAccount(account);
    return;
  }

  int              success = 0;
  list_t*          flows   = parseFlow(flow);
  list_node_t*     current_flow;
  list_iterator_t* it = list_iterator_new(flows, LIST_HEAD);
  while ((current_flow = list_iterator_next(it))) {
    if (strcaseequal(current_flow->val, FLOW_VALUE_REFRESH)) {
      if (getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL,
                                         pipes) != NULL) {
        success = 1;
        break;
      } else if (flows->len == 1) {
        ipc_writeOidcErrnoToPipe(pipes);
        list_iterator_destroy(it);
        list_destroy(flows);
        secFreeAccount(account);
        return;
      }
    } else if (strcaseequal(current_flow->val, FLOW_VALUE_PASSWORD)) {
      if (getAccessTokenUsingPasswordFlow(account, pipes) == OIDC_SUCCESS) {
        success = 1;
        break;
      } else if (flows->len == 1) {
        ipc_writeOidcErrnoToPipe(pipes);
        list_iterator_destroy(it);
        list_destroy(flows);
        secFreeAccount(account);
        return;
      }
    } else if (strcaseequal(current_flow->val, FLOW_VALUE_CODE) &&
               hasRedirectUris(account)) {
      initAuthCodeFlow(account, pipes, NULL, prioritizeCustom_str, arguments);
      list_iterator_destroy(it);
      list_destroy(flows);
      // secFreeAccount(account); //don't free it -> it is stored
      return;
    } else if (strcaseequal(current_flow->val, FLOW_VALUE_DEVICE)) {
      struct oidc_device_code* dc = initDeviceFlow(account);
      if (dc == NULL) {
        ipc_writeOidcErrnoToPipe(pipes);
        list_iterator_destroy(it);
        list_destroy(flows);
        secFreeAccount(account);
        return;
      }
      char* json = deviceCodeToJSON(*dc);
      ipc_writeToPipe(pipes, RESPONSE_ACCEPTED_DEVICE, json, account_json);
      secFree(json);
      secFreeDeviceCode(dc);
      list_iterator_destroy(it);
      list_destroy(flows);
      secFreeAccount(account);
      return;
    } else {  // UNKNOWN FLOW
      char* msg;
      if (strcaseequal(current_flow->val, FLOW_VALUE_CODE) &&
          !hasRedirectUris(account)) {
        msg = oidc_sprintf("Only '%s' flow specified, but no redirect uris",
                           FLOW_VALUE_CODE);
      } else {
        msg = oidc_sprintf("Unknown flow '%s'", (char*)current_flow->val);
      }
      ipc_writeToPipe(pipes, RESPONSE_ERROR, msg);
      secFree(msg);
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
  if (account_refreshTokenIsValid(account) && success) {
    char* json = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    db_addAccountEncrypted(account);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_ERROR,
                    success ? "OIDP response does not contain a refresh token"
                            : "No flow was successfull.");
    secFreeAccount(account);
  }
}

/**
 * checks if an account is feasable (issuer config / AT retrievable) and adds it
 * to the loaded list; does not check if account already loaded.
 */
oidc_error_t addAccount(struct ipcPipe pipes, struct oidc_account* account) {
  if (account == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  if (!strValid(account_getTokenEndpoint(account))) {
    return oidc_errno;
  }
  if (getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL, pipes) ==
      NULL) {
    return oidc_errno;
  }
  db_addAccountEncrypted(account);
  return OIDC_SUCCESS;
}

void oidcd_handleAdd(struct ipcPipe pipes, const char* account_json,
                     const char* timeout_str, const char* confirm_str) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Add request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  time_t timeout =
      strValid(timeout_str) ? atol(timeout_str) : agent_state.defaultTimeout;
  account_setDeath(account, timeout ? time(NULL) + timeout : 0);
  if (strToInt(confirm_str)) {
    account_setConfirmationRequired(account);
  }
  struct oidc_account* found = NULL;
  if ((found = db_getAccountDecrypted(account)) != NULL) {
    if (account_getDeath(found) != account_getDeath(account)) {
      account_setDeath(found, account_getDeath(account));
      char* msg = oidc_sprintf(
          "account already loaded. Lifetime set to %lu seconds.", timeout ?: 0);
      ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, msg);
      secFree(msg);
    } else {
      ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, "account already loaded.");
    }
    db_addAccountEncrypted(found);  // reencrypting sensitive data
    secFreeAccount(account);
    return;
  }
  if (addAccount(pipes, account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Loaded Account. Used timeout of %lu",
         timeout);
  if (timeout > 0) {
    char* msg = oidc_sprintf("Lifetime set to %lu seconds", timeout);
    ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, msg);
    secFree(msg);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
  }
}

void oidcd_handleDelete(struct ipcPipe pipes, const char* account_json) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Delete request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (revokeToken(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    char* error = oidc_sprintf("Could not revoke token: %s", oidc_serror());
    ipc_writeToPipe(pipes, RESPONSE_ERROR, error);
    secFree(error);
    return;
  }
  accountDB_removeIfFound(account);
  secFreeAccount(account);
  ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
}

void oidcd_handleRm(struct ipcPipe pipes, char* account_name) {
  if (account_name == NULL) {
    ipc_writeToPipe(
        pipes, RESPONSE_BADREQUEST,
        "Have to provide shortname of the account config that should be "
        "removed.");
    return;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Remove request for config '%s'",
         account_name);
  struct oidc_account key = {.shortname = account_name};
  if (accountDB_findValue(&key) == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
    return;
  }
  accountDB_removeIfFound(&key);
  ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
}

void oidcd_handleRemoveAll(struct ipcPipe pipes) {
  accountDB_reset();
  ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
}

oidc_error_t oidcd_autoload(struct ipcPipe pipes, char* short_name,
                            const char* application_hint) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Send autoload request for '%s'",
         short_name);
  char* res = ipc_communicateThroughPipe(pipes, INT_REQUEST_AUTOLOAD,
                                         short_name, application_hint ?: "");
  if (res == NULL) {
    return oidc_errno;
  }
  char* config = parseForConfig(res);
  if (config == NULL) {
    return oidc_errno;
  }
  struct oidc_account* account = getAccountFromJSON(config);
  account_setDeath(account, agent_state.defaultTimeout
                                ? time(NULL) + agent_state.defaultTimeout
                                : 0);
  if (addAccount(pipes, account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}

oidc_error_t oidcd_getConfirmation(struct ipcPipe pipes, char* short_name,
                                   const char* application_hint) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Send confirm request for '%s'", short_name);
  char* res = ipc_communicateThroughPipe(pipes, INT_REQUEST_CONFIRM, short_name,
                                         application_hint ?: "");
  if (res == NULL) {
    return oidc_errno;
  }
  oidc_errno = parseForErrorCode(res);
  return oidc_errno;
}

void oidcd_handleTokenIssuer(struct ipcPipe pipes, char* issuer,
                             const char* min_valid_period_str,
                             const char* scope, const char* application_hint,
                             const struct arguments* arguments) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Handle Token request from '%s' for issuer '%s'", application_hint,
         issuer);
  time_t min_valid_period =
      min_valid_period_str != NULL ? strToInt(min_valid_period_str) : 0;
  struct oidc_account* account  = NULL;
  list_t*              accounts = db_findAccountsByIssuerUrl(issuer);
  if (accounts == NULL) {
    // TODO
    if (arguments->no_autoload) {
      ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
      return;
    }
    // oidc_error_t autoload_error =
    //     oidcd_autoload(pipes, short_name, application_hint);
    // switch (autoload_error) {
    //   case OIDC_SUCCESS: account = db_getAccountDecrypted(&key); break;
    //   case OIDC_EUSRPWCNCL:
    //     ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
    //     return;
    //   default: ipc_writeOidcErrnoToPipe(pipes); return;
    // }
    oidc_errno = OIDC_NOTIMPL;
  } else if (accounts->len == 1) {
    if (arguments->confirm || account_getConfirmationRequired(account)) {
      if (oidcd_getConfirmation(
              pipes, issuer,
              application_hint) !=  // TODO new confirm message that makes the
                                    // realtionshipd between issuer and
                                    // shortname clear
          OIDC_SUCCESS) {
        ipc_writeOidcErrnoToPipe(pipes);
        return;
      }
    }
    account = _db_decryptFoundAccount(list_at(accounts, 0)->val);
  } else {  // more than 1 account loaded for this issuer
    // TODO
    oidc_errno = OIDC_NOTIMPL;
  }
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  char* access_token =
      getAccessTokenUsingRefreshFlow(account, min_valid_period, scope, pipes);
  db_addAccountEncrypted(account);  // reencrypting
  if (access_token == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token,
                  account_getIssuerUrl(account),
                  account_getTokenExpiresAt(account));
  if (strValid(scope)) {
    secFree(access_token);
  }
}

void oidcd_handleToken(struct ipcPipe pipes, char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char*             application_hint,
                       const struct arguments* arguments) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Token request from %s",
         application_hint);
  if (short_name == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR,
                    "Bad request. Required field '" IPC_KEY_SHORTNAME
                    "' not present.");
    return;
  }
  time_t min_valid_period =
      min_valid_period_str != NULL ? strToInt(min_valid_period_str) : 0;
  struct oidc_account* account = db_getAccountDecryptedByShortname(short_name);
  if (account == NULL) {
    if (arguments->no_autoload) {
      ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
      return;
    }
    oidc_error_t autoload_error =
        oidcd_autoload(pipes, short_name, application_hint);
    switch (autoload_error) {
      case OIDC_SUCCESS:
        account = db_getAccountDecryptedByShortname(short_name);
        break;
      case OIDC_EUSRPWCNCL:
        ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
        return;
      default: ipc_writeOidcErrnoToPipe(pipes); return;
    }
  } else if (arguments->confirm || account_getConfirmationRequired(account)) {
    if (oidcd_getConfirmation(pipes, short_name, application_hint) !=
        OIDC_SUCCESS) {
      db_addAccountEncrypted(account);  // reencrypting
      ipc_writeOidcErrnoToPipe(pipes);
      return;
    }
  }
  char* access_token =
      getAccessTokenUsingRefreshFlow(account, min_valid_period, scope, pipes);
  db_addAccountEncrypted(account);  // reencrypting
  if (access_token == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token,
                  account_getIssuerUrl(account),
                  account_getTokenExpiresAt(account));
  if (strValid(scope)) {
    secFree(access_token);
  }
}

void oidcd_handleRegister(struct ipcPipe pipes, const char* account_json,
                          const char* flows_json_str,
                          const char* access_token) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle Register request for flows: '%s'",
         flows_json_str);
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "daeSetByUser is: %d",
         issuer_getDeviceAuthorizationEndpointIsSetByUser(
             account_getIssuer(account)));
  if (NULL != accountDB_findValue(account)) {
    secFreeAccount(account);
    ipc_writeToPipe(
        pipes, RESPONSE_ERROR,
        "An account with this shortname is already loaded. I will not "
        "register a new one.");
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "daeSetByUser is: %d",
         issuer_getDeviceAuthorizationEndpointIsSetByUser(
             account_getIssuer(account)));
  list_t* flows = JSONArrayStringToList(flows_json_str);
  if (flows == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  char* res = dynamicRegistration(account, flows, access_token);
  if (res == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
  } else {
    if (!isJSONObject(res)) {
      char* escaped = escapeCharInStr(res, '"');
      ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO,
                      "Received no JSON formatted response.", escaped);
      secFree(escaped);
    } else {
      cJSON* json_res1 = stringToJson(res);
      if (jsonHasKey(json_res1, OIDC_KEY_ERROR)) {  // first failed
        list_removeIfFound(flows, findInList(flows, FLOW_VALUE_PASSWORD));
        char* res2 = dynamicRegistration(
            account, flows, access_token);  // TODO only try this if password
                                            // flow was in flow list
        if (res2 == NULL) {                 // second failed complety
          ipc_writeOidcErrnoToPipe(pipes);
        } else {
          if (jsonStringHasKey(res2,
                               OIDC_KEY_ERROR)) {  // first and second failed
            char* error = getJSONValue(json_res1, OIDC_KEY_ERROR_DESCRIPTION);
            if (error == NULL) {
              error = getJSONValue(json_res1, OIDC_KEY_ERROR);
            }
            ipc_writeToPipe(pipes, RESPONSE_ERROR, error);
            secFree(error);
          } else {  // first failed, second successful
            ipc_writeToPipe(pipes, RESPONSE_SUCCESS_CLIENT, res2);
          }
        }
        secFree(res2);
      } else {  // first was successfull
        char* scopes = getJSONValueFromString(res, OIDC_KEY_SCOPE);
        if (!strSubStringCase(scopes, OIDC_SCOPE_OPENID) ||
            !strSubStringCase(scopes, OIDC_SCOPE_OFFLINE_ACCESS)) {
          // did not get all scopes necessary for oidc-agent
          oidc_errno = OIDC_EUNSCOPE;
          ipc_writeToPipe(pipes, RESPONSE_ERROR_CLIENT, oidc_serror(), res);
        }
        secFree(scopes);
        ipc_writeToPipe(pipes, RESPONSE_SUCCESS_CLIENT, res);
      }
      secFreeJson(json_res1);
    }
  }
  list_destroy(flows);
  secFree(res);
  secFreeAccount(account);
}

void oidcd_handleCodeExchange(struct ipcPipe pipes, const char* redirected_uri,
                              const char* fromString) {
  if (redirected_uri == NULL) {
    oidc_setArgNullFuncError(__func__);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  int fromGen = strToInt(fromString);
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Handle codeExchange request for redirect_uri '%s' from %s",
         redirected_uri, fromGen ? "oidc-gen" : "other (httpserver)");
  struct codeState codeState    = codeStateFromURI(redirected_uri);
  char*            redirect_uri = codeState.uri;
  char*            state        = codeState.state;
  char*            code         = codeState.code;
  if (state == NULL || code == NULL || redirect_uri == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    return;
  }
  struct codeExchangeEntry key = {.state = state};
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Getting code_verifier and account info for state '%s'", state);
  struct codeExchangeEntry* cee = codeVerifierDB_findValue(&key);
  if (cee == NULL) {
    oidc_errno = OIDC_EWRONGSTATE;
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    return;
  }

  struct oidc_account* account = cee->account;
  if (account == NULL) {
    oidc_setInternalError("account found for state is NULL");
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    secFreeCodeExchangeContent(cee);
    codeVerifierDB_removeIfFound(cee);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    secFreeCodeExchangeContent(cee);
    codeVerifierDB_removeIfFound(cee);
    return;
  }
  // syslog(LOG_AUTHPRIV | LOG_DEBUG, "code_verifier for state '%s' is '%s'",
  //        state, cee->code_verifier);
  if (getAccessTokenUsingAuthCodeFlow(account, code, redirect_uri,
                                      cee->code_verifier,
                                      pipes) != OIDC_SUCCESS) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    // secFreeCodeExchangeContent(cee);
    // codeVerifierDB_removeIfFound(cee);
    return;
  }
  if (account_refreshTokenIsValid(account)) {
    char* json = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    secFreeCodeState(codeState);
    db_addAccountEncrypted(account);
    secFree(cee->code_verifier);
    if (fromGen) {
      termHttpServer(cee->state);
      account->usedStateChecked = 1;
    }
    account_setUsedState(account, cee->state);
    codeVerifierDB_removeIfFound(cee);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "Could not get a refresh token");
    secFreeCodeState(codeState);
    secFreeCodeExchangeContent(cee);
    codeVerifierDB_removeIfFound(cee);
  }
}

void oidcd_handleDeviceLookup(struct ipcPipe pipes, const char* account_json,
                              const char* device_json) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle deviceLookup request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  struct oidc_device_code* dc = getDeviceCodeFromJSON(device_json);
  if (dc == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeAccount(account);
    return;
  }
  if (getIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeDeviceCode(dc);
    return;
  }
  if (getAccessTokenUsingDeviceFlow(account, oidc_device_getDeviceCode(*dc),
                                    pipes) != OIDC_SUCCESS) {
    secFreeAccount(account);
    secFreeDeviceCode(dc);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  secFreeDeviceCode(dc);
  if (account_refreshTokenIsValid(account)) {
    char* json = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    db_addAccountEncrypted(account);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "Could not get a refresh token");
    secFreeAccount(account);
  }
}

void oidcd_handleStateLookUp(struct ipcPipe pipes, char* state) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle stateLookUp request");
  struct oidc_account key = {.usedState = state};
  matchFunction       oldMatch =
      accountDB_setMatchFunction((matchFunction)account_matchByState);
  struct oidc_account* account = db_getAccountDecrypted(&key);
  accountDB_setMatchFunction(oldMatch);
  if (account == NULL) {
    char* info =
        oidc_sprintf("No loaded account info found for state=%s", state);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_INFO, STATUS_NOTFOUND, info);
    secFree(info);
    return;
  }
  if (account->usedStateChecked) {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_INFO, STATUS_FOUNDBUTDONE,
                    "Account config already retrieved from another oidc-gen");
    db_addAccountEncrypted(account);  // reencrypting
    return;
  }
  account->usedStateChecked = 1;
  char* config              = accountToJSONString(account);
  ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, config);
  secFree(config);
  db_addAccountEncrypted(account);  // reencrypting
  termHttpServer(state);
}

void oidcd_handleTermHttp(struct ipcPipe pipes, const char* state) {
  termHttpServer(state);
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS);
}

void oidcd_handleLock(struct ipcPipe pipes, const char* password, int _lock) {
  if (_lock) {
    if (lock(password) == OIDC_SUCCESS) {
      ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, "Agent locked");
      return;
    }
  } else {
    if (unlock(password) == OIDC_SUCCESS) {
      ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, "Agent unlocked");
      return;
    }
  }
  ipc_writeOidcErrnoToPipe(pipes);
}
