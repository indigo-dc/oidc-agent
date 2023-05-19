#include "oidcd_handler.h"
#ifdef __MSYS__
#include <sys/select.h>
#endif

#include <string.h>
#include <strings.h>
#include <time.h>
#include <utils/pass.h>

#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "defines/version.h"
#include "deviceCodeEntry.h"
#include "internal_request_handler.h"
#include "ipc/pipe.h"
#include "ipc/serveripc.h"
#include "oidc-agent/agent_state.h"
#include "oidc-agent/httpserver/termHttpserver.h"
#include "oidc-agent/mytoken/oidc_flow.h"
#include "oidc-agent/mytoken/submytoken.h"
#include "oidc-agent/oidc/device_code.h"
#include "oidc-agent/oidc/flows/access_token_handler.h"
#include "oidc-agent/oidc/flows/code.h"
#include "oidc-agent/oidc/flows/deleteClient.h"
#include "oidc-agent/oidc/flows/device.h"
#include "oidc-agent/oidc/flows/openid_config.h"
#include "oidc-agent/oidc/flows/registration.h"
#include "oidc-agent/oidc/flows/revoke.h"
#include "oidc-agent/oidc/oidc_agent_help.h"
#include "oidc-agent/oidcd/codeExchangeEntry.h"
#include "oidc-agent/oidcd/parse_internal.h"
#include "utils/accountUtils.h"
#include "utils/agentLogger.h"
#include "utils/config/agent_config.h"
#include "utils/config/gen_config.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/dbCryptUtils.h"
#include "utils/db/account_db.h"
#include "utils/db/codeVerifier_db.h"
#include "utils/db/deviceCode_db.h"
#include "utils/db/file_db.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/oidc/oidcUtils.h"
#include "utils/parseJson.h"
#include "utils/string/stringUtils.h"
#include "utils/uriUtils.h"

void initAuthCodeFlow(struct oidc_account* account, struct ipcPipe pipes,
                      const char* info, const char* nowebserver_str,
                      const char* noscheme_str, const unsigned char only_at,
                      const struct arguments* arguments) {
  if (arguments->no_webserver || strToInt(nowebserver_str)) {
    account_setNoWebServer(account);
  }
  if (arguments->no_scheme || strToInt(noscheme_str)) {
    account_setNoScheme(account);
  }
  size_t state_len       = 24;
  size_t socket_path_len = oidc_strlen(getServerSocketPath());
  char*  socket_path_base64 =
      toBase64UrlSafe(getServerSocketPath(), socket_path_len);
  // agent_log(DEBUG, "Base64 socket path is '%s'",
  //        socket_path_base64);
  char random[state_len + 1];
  randomFillBase64UrlSafe(random, state_len);
  random[state_len] = '\0';
  char*  state      = oidc_sprintf("%d%s:%lu:%s", only_at ? 1 : 0, random,
                                   socket_path_len, socket_path_base64);
  char** state_ptr  = &state;
  secFree(socket_path_base64);

  char* code_verifier = secAlloc(CODE_VERIFIER_LEN + 1);
  randomFillBase64UrlSafe(code_verifier, CODE_VERIFIER_LEN);

  char* uri = buildCodeFlowUri(account, state_ptr, &code_verifier, only_at);
  if (uri == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFree(code_verifier);
    secFree(*state_ptr);
    secFreeAccount(account);
    return;
  }
  // agent_log(DEBUG, "code_verifier for state '%s' is '%s'",
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

void _handleGenFlows(struct ipcPipe pipes, struct oidc_account* account,
                     const char* flow, const char* scope,
                     const unsigned char only_at, const char* nowebserver_str,
                     const char*             noscheme_str,
                     const struct arguments* arguments) {
  int              success = 0;
  list_t*          flows   = parseFlow(flow);
  list_node_t*     current_flow;
  list_iterator_t* it            = list_iterator_new(flows, LIST_HEAD);
  unsigned int     numberOfFlows = flows->len;
  unsigned int     flowsTried    = 0;
  while ((current_flow = list_iterator_next(it))) {
    flowsTried++;
    if (strcaseequal(current_flow->val, FLOW_VALUE_REFRESH)) {
      char* at = NULL;
      if ((at = getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, scope,
                                               account_getAudience(account),
                                               pipes)) != NULL) {
        success = 1;
        if (only_at) {
          account_setAccessToken(
              account,
              at);  // if only_at store the AT so we can send it back, we have
                    // to store it manually because we provided manual scopes
                    // (because offline_access removed). The token will be
                    // correctly freed at the end when the account is freed.
        }
        break;
      } else if (flows->len == 1) {
        ipc_writeOidcErrnoToPipe(pipes);
        list_iterator_destroy(it);
        secFreeList(flows);
        secFreeAccount(account);
        return;
      }
    } else if (strcaseequal(current_flow->val, FLOW_VALUE_PASSWORD)) {
      if (getAccessTokenUsingPasswordFlow(account, FORCE_NEW_TOKEN, scope,
                                          pipes) == OIDC_SUCCESS) {
        success = 1;
        break;
      } else if (flows->len == 1) {
        ipc_writeOidcErrnoToPipe(pipes);
        list_iterator_destroy(it);
        secFreeList(flows);
        secFreeAccount(account);
        return;
      }
    } else if (strcaseequal(current_flow->val, FLOW_VALUE_CODE) &&
               hasRedirectUris(account)) {
      initAuthCodeFlow(account, pipes, NULL, nowebserver_str, noscheme_str,
                       only_at, arguments);
      list_iterator_destroy(it);
      secFreeList(flows);
      // secFreeAccount(account); //don't free it -> it is stored
      return;
    } else if (strcaseequal(current_flow->val, FLOW_VALUE_DEVICE) ||
               strcaseequal(current_flow->val, FLOW_VALUE_MT_OIDC)) {
      if (scope) {
        account_setScopeExact(account, oidc_strcopy(scope));
      }
      struct oidc_device_code* dc =
          strcaseequal(current_flow->val, FLOW_VALUE_MT_OIDC)
              ? initMytokenOIDCFlow(account)
              : initDeviceFlow(account);
      if (dc == NULL) {
        if (flowsTried < numberOfFlows) {
          continue;
        }
        ipc_writeOidcErrnoToPipe(pipes);
        list_iterator_destroy(it);
        secFreeList(flows);
        secFreeAccount(account);
        return;
      }
      char* json = deviceCodeToJSON(*dc);
      ipc_writeToPipe(pipes, RESPONSE_ACCEPTED_DEVICE, json);
      secFree(json);
      secFreeDeviceCode(dc);
      list_iterator_destroy(it);
      secFreeList(flows);
      // secFreeAccount(account); // Don't free account, it is stored
      return;
    } else {
      char* msg;
      if (strcaseequal(current_flow->val, FLOW_VALUE_CODE) &&
          !hasRedirectUris(account)) {
        if (flowsTried < numberOfFlows) {
          continue;
        }
        msg = oidc_sprintf("Only '%s' flow specified, but no redirect uris",
                           FLOW_VALUE_CODE);
      } else {  // UNKNOWN FLOW
        msg = oidc_sprintf("Unknown flow '%s'", (char*)current_flow->val);
      }
      ipc_writeToPipe(pipes, RESPONSE_ERROR, msg);
      secFree(msg);
      list_iterator_destroy(it);
      secFreeList(flows);
      secFreeAccount(account);
      return;
    }
  }

  list_iterator_destroy(it);
  secFreeList(flows);

  account_setUsername(account, NULL);
  account_setPassword(account, NULL);
  if (success && account_refreshTokenIsValid(account) && !only_at) {
    oidcd_handleUpdateIssuer(pipes, account_getIssuerUrl(account),
                             account_getName(account), INT_ACTION_VALUE_ADD);
    char* json = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    db_addAccountEncrypted(account);
  } else if (success && only_at && strValid(account_getAccessToken(account))) {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS,
                    account_getAccessToken(account),
                    account_getIssuerUrl(account),
                    account_getTokenExpiresAt(account));
    secFreeAccount(account);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_ERROR,
                    success && !only_at
                        ? "OIDP response does not contain a refresh token"
                        : "No flow was successful.");
    secFreeAccount(account);
  }
}

void oidcd_handleGen(struct ipcPipe pipes, const char* account_json,
                     const char* flow, const char* nowebserver_str,
                     const char* noscheme_str, const char* only_at_str,
                     const struct arguments* arguments) {
  agent_log(DEBUG, "Handle Gen request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (obtainIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (!strValid(account_getTokenEndpoint(account))) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeAccount(account);
    return;
  }

  const int only_at = strToInt(only_at_str);
  char* scope = only_at ? removeScope(oidc_strcopy(account_getScope(account)),
                                      OIDC_SCOPE_OFFLINE_ACCESS)
                        : NULL;

  _handleGenFlows(pipes, account, flow, scope, only_at, nowebserver_str,
                  noscheme_str, arguments);
  secFree(scope);
}

/**
 * checks if an account is feasible (issuer config / AT retrievable) and adds it
 * to the loaded list; does not check if account already loaded.
 */
oidc_error_t addAccount(struct ipcPipe pipes, struct oidc_account* account) {
  if (account == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  if (obtainIssuerConfig(account) != OIDC_SUCCESS) {
    return oidc_errno;
  }
  if (!strValid(account_getTokenEndpoint(account))) {
    return oidc_errno;
  }
  if (getAccessTokenUsingRefreshFlow(account, FORCE_NEW_TOKEN, NULL, NULL,
                                     pipes) == NULL) {
    account_setDeath(account,
                     time(NULL) + 10);  // with short timeout so no password
                                        // required for re-authentication
    db_addAccountEncrypted(account);
    return oidc_errno;
  }
  db_addAccountEncrypted(account);
  oidcd_handleUpdateIssuer(pipes, account_getIssuerUrl(account),
                           account_getName(account), INT_ACTION_VALUE_ADD);
  return OIDC_SUCCESS;
}

void oidcd_handleAdd(struct ipcPipe pipes, const char* account_json,
                     const char* timeout_str, const char* confirm_str,
                     const char* alwaysallowid) {
  agent_log(DEBUG, "Handle Add request");
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
  if (strToInt(alwaysallowid)) {
    account_setAlwaysAllowId(account);
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
    char* help = getHelpWithAccountInfo(account);
    if (help != NULL) {
      ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO, oidc_serror(), help);
      secFree(help);
      return;
    }
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  agent_log(DEBUG, "Loaded Account. Used timeout of %lu", timeout);
  if (timeout > 0) {
    char* msg = oidc_sprintf("Lifetime set to %lu seconds", timeout);
    ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, msg);
    secFree(msg);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
  }
}

void oidcd_handleDeleteClient(struct ipcPipe pipes, const char* client_uri,
                              const char* registration_access_token,
                              const char* cert_path) {
  agent_log(DEBUG, "Handle DeleteClient request");
  if (deleteClient(client_uri, registration_access_token, cert_path) !=
      OIDC_SUCCESS) {
    char* error = oidc_sprintf("Could not delete client: %s", oidc_serror());
    ipc_writeToPipe(pipes, RESPONSE_ERROR, error);
    secFree(error);
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
}

void oidcd_handleDelete(struct ipcPipe pipes, const char* account_json) {
  agent_log(DEBUG, "Handle Delete request");
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  if (obtainIssuerConfig(account) != OIDC_SUCCESS) {
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
  oidcd_handleUpdateIssuer(pipes, account_getIssuerUrl(account),
                           account_getName(account), INT_ACTION_VALUE_REMOVE);
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
  agent_log(DEBUG, "Handle Remove request for config '%s'", account_name);
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

oidc_error_t oidcd_autoload(struct ipcPipe pipes, const char* short_name,
                            const char* issuer, const char* application_hint) {
  agent_log(DEBUG, "Send autoload request for '%s'", short_name);
  char* res =
      issuer ? ipc_communicateThroughPipe(
                   pipes, INT_REQUEST_AUTOLOAD_WITH_ISSUER, short_name, issuer,
                   application_hint ?: "")
             : ipc_communicateThroughPipe(pipes, INT_REQUEST_AUTOLOAD,
                                          short_name, application_hint ?: "");
  if (res == NULL) {
    return oidc_errno;
  }
  char* config = parseForConfig(res);
  if (config == NULL) {
    return oidc_errno;
  }
  struct oidc_account* account = getAccountFromJSON(config);
  secFree(config);
  account_setDeath(account, agent_state.defaultTimeout
                                ? time(NULL) + agent_state.defaultTimeout
                                : 0);
  return addAccount(pipes, account);
}

#define CONFIRMATION_MODE_AT 0
#define CONFIRMATION_MODE_ID 1

oidc_error_t _oidcd_getConfirmation(unsigned char mode, struct ipcPipe pipes,
                                    const char* short_name, const char* issuer,
                                    const char* application_hint) {
  agent_log(DEBUG, "Send confirm request for '%s'", short_name);
  const char* request_type = NULL;
  switch (mode) {
    case CONFIRMATION_MODE_AT: request_type = INT_REQUEST_VALUE_CONFIRM; break;
    case CONFIRMATION_MODE_ID:
      request_type = INT_REQUEST_VALUE_CONFIRMIDTOKEN;
      break;
  }
  char* res = issuer ? ipc_communicateThroughPipe(
                           pipes, INT_REQUEST_CONFIRM_WITH_ISSUER, request_type,
                           issuer, short_name, application_hint ?: "")
                     : ipc_communicateThroughPipe(pipes, INT_REQUEST_CONFIRM,
                                                  request_type, short_name,
                                                  application_hint ?: "");
  if (res == NULL) {
    return oidc_errno;
  }
  oidc_errno = parseForErrorCode(res);
  return oidc_errno;
}

oidc_error_t oidcd_getConfirmation(struct ipcPipe pipes, const char* short_name,
                                   const char* issuer,
                                   const char* application_hint) {
  return _oidcd_getConfirmation(CONFIRMATION_MODE_AT, pipes, short_name, issuer,
                                application_hint);
}

oidc_error_t oidcd_getIdTokenConfirmation(struct ipcPipe pipes,
                                          const char*    short_name,
                                          const char*    issuer,
                                          const char*    application_hint) {
  return _oidcd_getConfirmation(CONFIRMATION_MODE_ID, pipes, short_name, issuer,
                                application_hint);
}

char* _oidcd_getMytokenConfirmation(struct ipcPipe pipes,
                                    const char*    base64html) {
  agent_log(DEBUG, "Send mytoken confirm request");
  char* res = ipc_communicateThroughPipe(pipes, INT_REQUEST_CONFIRM_MYTOKEN,
                                         base64html);
  if (res == NULL) {
    return NULL;
  }
  oidc_errno = parseForErrorCode(oidc_strcopy(res));
  if (oidc_errno != OIDC_SUCCESS) {
    secFree(res);
    return NULL;
  }
  return parseForInfo(res);
}

char* oidcd_queryDefaultAccountIssuer(struct ipcPipe pipes,
                                      const char*    issuer) {
  agent_log(DEBUG, "Send default account config query request for issuer '%s'",
            issuer);
  char* res = ipc_communicateThroughPipe(
      pipes, INT_REQUEST_QUERY_ACCDEFAULT_ISSUER, issuer);
  if (res == NULL) {
    return NULL;
  }
  char* shortname = getJSONValueFromString(res, IPC_KEY_SHORTNAME);
  secFree(res);
  if (!strValid(shortname)) {
    secFree(shortname);
    return NULL;
  }
  return shortname;
}

struct oidc_account* _getLoadedUnencryptedAccount(
    struct ipcPipe pipes, const char* short_name, const char* application_hint,
    const struct arguments* arguments) {
  struct oidc_account* account = db_getAccountDecryptedByShortname(short_name);
  if (account) {
    return account;
  }
  if (arguments->no_autoload) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
    return NULL;
  }
  oidc_error_t autoload_error =
      oidcd_autoload(pipes, short_name, NULL, application_hint);
  switch (autoload_error) {
    case OIDC_SUCCESS:
      account = db_getAccountDecryptedByShortname(short_name);
      if (account == NULL) {
        ipc_writeOidcErrnoToPipe(pipes);
      }
      return account;
    case OIDC_EUSRPWCNCL:
      ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
      return NULL;
    default: {
      const char* help = getHelp();
      if (help != NULL) {
        char* h = strreplace(help, "<shortname>", short_name);
        ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO, oidc_serror(), h);
        secFree(h);
        return NULL;
      }
      ipc_writeOidcErrnoToPipe(pipes);
      return NULL;
    }
  }
}

struct oidc_account* _getLoadedUnencryptedAccountForIssuer(
    struct ipcPipe pipes, const char* issuer, const char* scope,
    const char* application_hint, const struct arguments* arguments) {
  struct oidc_account* account  = NULL;
  list_t*              accounts = db_findAccountsByIssuerUrl(issuer);
  if (accounts == NULL) {  // no accounts loaded for this issuer
    if (arguments->no_autoload) {
      ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
      return NULL;
    }
    char* defaultAccount = oidcd_queryDefaultAccountIssuer(pipes, issuer);
    if (defaultAccount == NULL) {
      if (getAgentConfig()->autogen) {
        const char* scopes = AGENT_SCOPE_ALL;
        switch (getAgentConfig()->autogenscopemode) {
          case AGENTCONFIG_AUTOGENSCOPEMODE_EXACT: scopes = scope;
        }

        agent_log(DEBUG, "Send autogen request for '%s'", issuer);
        ipc_writeToPipe(pipes, INT_REQUEST_AUTOGEN, issuer, scopes,
                        application_hint ?: "");
        // we abort the current request and oidcp will start a gen
        // flow and after success resend the original request
        return NULL;
      }
    }
    oidc_error_t autoload_error =
        oidcd_autoload(pipes, defaultAccount, issuer, application_hint);
    switch (autoload_error) {
      case OIDC_SUCCESS:
        account = db_getAccountDecryptedByShortname(defaultAccount);
        secFree(defaultAccount);
        return account;
      case OIDC_EUSRPWCNCL:
        ipc_writeToPipe(pipes, RESPONSE_ERROR, ACCOUNT_NOT_LOADED);
        secFree(defaultAccount);
        return NULL;
      default: {
        const char* help = getHelp();
        if (help != NULL) {
          char* h   = strreplace(help, "<shortname>", defaultAccount);
          char* tmp = strreplace(h, "<issuer>", issuer);
          secFree(h);
          h = tmp;
          ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO, oidc_serror(), h);
          secFree(h);
          return NULL;
        }
        ipc_writeOidcErrnoToPipe(pipes);
        return NULL;
      }
    }
  } else if (accounts->len ==
             1) {  // only one account loaded for this issuer -> use this one
    account = _db_decryptFoundAccount(list_at(accounts, 0)->val);
    secFreeList(accounts);
  } else {  // more than 1 account loaded for this issuer
    char* defaultAccount = oidcd_queryDefaultAccountIssuer(pipes, issuer);
    account              = db_getAccountDecryptedByShortname(defaultAccount);
    if (account == NULL) {
      account = _db_decryptFoundAccount(
          list_at(accounts, accounts->len - 1)
              ->val);  // use the account that was loaded last
    }
    secFreeList(accounts);
  }

  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return NULL;
  }
  if (arguments->confirm || account_getConfirmationRequired(account)) {
    if (oidcd_getConfirmation(pipes, account_getName(account), issuer,
                              application_hint) != OIDC_SUCCESS) {
      db_addAccountEncrypted(account);  // reencrypting
      ipc_writeOidcErrnoToPipe(pipes);
      return NULL;
    }
  }
  return account;
}

void oidcd_handleTokenIssuer(struct ipcPipe pipes, const char* issuer,
                             const char* min_valid_period_str,
                             const char* scope, const char* application_hint,
                             const char*             audience,
                             const struct arguments* arguments) {
  agent_log(DEBUG, "Handle Token request from '%s' for issuer '%s'",
            application_hint, issuer);
  time_t min_valid_period =
      min_valid_period_str != NULL ? strToInt(min_valid_period_str) : 0;
  struct oidc_account* account = _getLoadedUnencryptedAccountForIssuer(
      pipes, issuer, scope, application_hint, arguments);
  if (account == NULL) {
    return;
  }
  char* access_token = getAccessTokenUsingRefreshFlow(account, min_valid_period,
                                                      scope, audience, pipes);
  if (access_token == NULL) {
    char* help = getHelpWithAccountInfo(account);
    db_addAccountEncrypted(account);  // reencrypting
    if (help != NULL) {
      ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO, oidc_serror(), help);
      secFree(help);
      return;
    }
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  db_addAccountEncrypted(account);  // reencrypting
  ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token,
                  account_getIssuerUrl(account),
                  account_getTokenExpiresAt(account));
  if (strValid(scope)) {
    secFree(access_token);
  }
}

void oidcd_handleReauthenticate(struct ipcPipe pipes, char* short_name,
                                const struct arguments* arguments) {
  agent_log(DEBUG, "Handle Reauthentication request");
  if (short_name == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR,
                    "Bad request. Required field '" IPC_KEY_SHORTNAME
                    "' not present.");
    return;
  }
  struct oidc_account* account =
      _getLoadedUnencryptedAccount(pipes, short_name, NULL, arguments);
  if (account == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "Could not get account config");
    return;
  }
  // The account we just retrieved has a very short timeout and will be freed
  // soon if we don't increase it
  account_setDeath(account,
                   arguments->lifetime ? arguments->lifetime + time(NULL) : 0);
  _handleGenFlows(pipes, account,
                  account_getMytokenUrl(account)
                      ? "[\"" FLOW_VALUE_MT_OIDC "\"]"
                      : "[\"" FLOW_VALUE_PASSWORD "\",\"" FLOW_VALUE_DEVICE
                        "\",\"" FLOW_VALUE_CODE "\"]",
                  NULL, 0, NULL, "1", arguments);
}

void oidcd_handleToken(struct ipcPipe pipes, const char* short_name,
                       const char* min_valid_period_str, const char* scope,
                       const char* application_hint, const char* audience,
                       const struct arguments* arguments) {
  agent_log(DEBUG, "Handle Token request from %s", application_hint);
  if (short_name == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR,
                    "Bad request. Required field '" IPC_KEY_SHORTNAME
                    "' not present.");
    return;
  }
  time_t min_valid_period =
      min_valid_period_str != NULL ? strToInt(min_valid_period_str) : 0;
  struct oidc_account* account = _getLoadedUnencryptedAccount(
      pipes, short_name, application_hint, arguments);
  if (account == NULL) {
    return;
  }
  if (arguments->confirm || account_getConfirmationRequired(account)) {
    if (oidcd_getConfirmation(pipes, short_name, NULL, application_hint) !=
        OIDC_SUCCESS) {
      db_addAccountEncrypted(account);  // reencrypting
      ipc_writeOidcErrnoToPipe(pipes);
      return;
    }
  }
  char* access_token = getAccessTokenUsingRefreshFlow(account, min_valid_period,
                                                      scope, audience, pipes);
  if (access_token == NULL) {
    char* help = getHelpWithAccountInfo(account);
    db_addAccountEncrypted(account);  // reencrypting
    if (help != NULL) {
      ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO, oidc_serror(), help);
      secFree(help);
      return;
    }
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  db_addAccountEncrypted(account);  // reencrypting
  ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS, access_token,
                  account_getIssuerUrl(account),
                  account_getTokenExpiresAt(account));
  if (strValid(scope)) {
    secFree(access_token);
  }
}

void oidcd_handleMytoken(struct ipcPipe pipes, const char* short_name,
                         const char* profile, const char* application_hint,
                         const struct arguments* arguments) {
  agent_log(DEBUG, "Handle Mytoken request from %s", application_hint);
  if (short_name == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR,
                    "Bad request. Required field '" IPC_KEY_SHORTNAME
                    "' not present.");
    return;
  }
  struct oidc_account* account = _getLoadedUnencryptedAccount(
      pipes, short_name, application_hint, arguments);
  if (account == NULL) {
    return;
  }
  char* res = get_submytoken(pipes, account, profile, application_hint);
  db_addAccountEncrypted(account);  // reencrypting
  if (res == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  ipc_writeToPipe(pipes, res);
  secFree(res);
}

void oidcd_handleIdToken(struct ipcPipe pipes, const char* short_name,
                         const char* issuer, const char* scope,
                         const char*             application_hint,
                         const struct arguments* arguments) {
  agent_log(DEBUG, "Handle ID-Token request from %s", application_hint);
  struct oidc_account* account =
      short_name != NULL
          ? _getLoadedUnencryptedAccount(pipes, short_name, application_hint,
                                         arguments)
          : _getLoadedUnencryptedAccountForIssuer(pipes, issuer, scope,
                                                  application_hint, arguments);
  if (account == NULL) {
    return;
  }
  if (!(arguments->always_allow_idtoken ||
        account_getAlwaysAllowId(
            account))) {  // TODO account based does not work yet
    if (oidcd_getIdTokenConfirmation(pipes, short_name, issuer,
                                     application_hint) != OIDC_SUCCESS) {
      ipc_writeOidcErrnoToPipe(pipes);
      return;
    }
  }
  char* id_token = getIdToken(account, scope, pipes);
  db_addAccountEncrypted(account);  // reencrypting
  if (id_token == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_STATUS_IDTOKEN, STATUS_SUCCESS, id_token,
                  account_getIssuerUrl(account));
  secFree(id_token);
}

void oidcd_handleRegister(struct ipcPipe pipes, const char* account_json,
                          const char* flows_json_str,
                          const char* access_token) {
  agent_log(DEBUG, "Handle Register request for flows: '%s'", flows_json_str);
  struct oidc_account* account = getAccountFromJSON(account_json);
  if (account == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  agent_log(DEBUG, "daeSetByUser is: %d",
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
  if (obtainIssuerConfig(account) != OIDC_SUCCESS) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  agent_log(DEBUG, "daeSetByUser is: %d",
            issuer_getDeviceAuthorizationEndpointIsSetByUser(
                account_getIssuer(account)));
  list_t* flows = JSONArrayStringToList(flows_json_str);
  if (flows == NULL) {
    secFreeAccount(account);
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }

  char* res = dynamicRegistration(account, flows, access_token);
  secFreeList(flows);
  if (res == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeAccount(account);
    return;
  }
  if (!isJSONObject(res)) {
    char* escaped = escapeCharInStr(res, '"');
    ipc_writeToPipe(pipes, RESPONSE_ERROR_INFO,
                    "Received no JSON formatted response.", escaped);
    secFree(escaped);
    secFreeAccount(account);
    return;
  }
  cJSON* json_res = stringToJson(res);
  if (jsonHasKey(json_res, OIDC_KEY_ERROR)) {
    char* error = parseForError(res);  // frees res
    ipc_writeToPipe(pipes, RESPONSE_ERROR, error);
    secFree(error);
    secFreeAccount(account);
    return;
  }
  char* scopes = getJSONValue(json_res, OIDC_KEY_SCOPE);
  secFreeJson(json_res);
  if ((!strSubStringCase(scopes, OIDC_SCOPE_OPENID) &&
       !account_getIsOAuth2(account)) ||
      !strSubStringCase(scopes, OIDC_SCOPE_OFFLINE_ACCESS)) {
    // did not get all scopes necessary for oidc-agent
    oidc_errno = OIDC_EUNSCOPE;
    ipc_writeToPipe(pipes, RESPONSE_ERROR_CLIENT, oidc_serror(), res);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_SUCCESS_CLIENT_MAXSCOPES, res,
                    account_getScopesSupported(account));
  }
  secFree(scopes);
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
  agent_log(DEBUG, "Handle codeExchange request for redirect_uri '%s' from %s",
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
  if (strlen(state) < 3) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "state malformed");
    secFreeCodeState(codeState);
    return;
  }
  const unsigned char      only_at = state[2] == '1' ? 1 : 0;
  struct codeExchangeEntry key     = {.state = state};
  agent_log(DEBUG, "Getting code_verifier and account info for state '%s'",
            state);
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
  if (obtainIssuerConfig(account) != OIDC_SUCCESS) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    secFreeCodeExchangeContent(cee);
    codeVerifierDB_removeIfFound(cee);
    return;
  }
  // agent_log(DEBUG, "code_verifier for state '%s' is '%s'",
  //        state, cee->code_verifier);
  if (getAccessTokenUsingAuthCodeFlow(account, code, redirect_uri,
                                      cee->code_verifier, FORCE_NEW_TOKEN,
                                      pipes) != OIDC_SUCCESS) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeCodeState(codeState);
    return;
  }
  if (account_refreshTokenIsValid(account) && !only_at) {
    oidcd_handleUpdateIssuer(pipes, account_getIssuerUrl(account),
                             account_getName(account), INT_ACTION_VALUE_ADD);
    char* json = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    if (fromGen) {
      termHttpServer(cee->state);
      account->usedStateChecked = 1;
    }
    db_addAccountEncrypted(account);
    secFree(cee->code_verifier);
    account_setUsedState(account, cee->state);
  } else if (only_at && strValid(account_getAccessToken(account))) {
    agent_log(NOTICE, "HELLO IM SENDING THE AT NOW");
    ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS,
                    account_getAccessToken(account),
                    account_getIssuerUrl(account),
                    account_getTokenExpiresAt(account));
    if (fromGen) {
      termHttpServer(cee->state);
      account->usedStateChecked = 1;
    }
    db_addAccountEncrypted(account);
    secFree(cee->code_verifier);
    account_setUsedState(account, cee->state);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "Could not get a refresh token");
    secFreeCodeExchangeContent(cee);
  }
  secFreeCodeState(codeState);
  codeVerifierDB_removeIfFound(cee);
}

void oidcd_handleDeviceLookup(struct ipcPipe pipes, const char* device_json,
                              const char* only_at_str) {
  agent_log(DEBUG, "Handle deviceLookup request");
  struct oidc_device_code* dc = getDeviceCodeFromJSON(device_json);
  if (dc == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  struct deviceCodeEntry key = {.device_code = dc->device_code};
  agent_log(DEBUG, "Getting account for device_code '%s'", dc->device_code);
  struct deviceCodeEntry* dce = deviceCodeDB_findValue(&key);
  if (dce == NULL) {
    oidc_errno = OIDC_EWRONGDEVICECODE;
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }

  struct oidc_account* account = dce->account;
  if (account == NULL) {
    oidc_setInternalError("account found for device code is NULL");
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeDeviceCodeEntryContent(dce);
    deviceCodeDB_removeIfFound(dce);
    return;
  }
  if (obtainIssuerConfig(account) != OIDC_SUCCESS) {
    ipc_writeOidcErrnoToPipe(pipes);
    secFreeDeviceCode(dc);
    secFreeDeviceCodeEntryContent(dce);
    deviceCodeDB_removeIfFound(dce);
    return;
  }
  if (account_getMytokenUrl(account)
          ? lookUpMytokenPollingCode(account, oidc_device_getDeviceCode(*dc),
                                     pipes)
          : getAccessTokenUsingDeviceFlow(
                account, oidc_device_getDeviceCode(*dc), FORCE_NEW_TOKEN,
                pipes) != OIDC_SUCCESS) {
    secFreeDeviceCode(dc);
    if ((oidc_errno == OIDC_EOIDC || oidc_errno == OIDC_EMYTOKEN) &&
        (strequal(oidc_serror(), OIDC_SLOW_DOWN) ||
         strequal(oidc_serror(), OIDC_AUTHORIZATION_PENDING))) {
      pass;
    } else {
      secFreeDeviceCodeEntryContent(dce);
      deviceCodeDB_removeIfFound(dce);
    }
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  secFreeDeviceCode(dc);
  const int only_at = strToInt(only_at_str);
  if (account_refreshTokenIsValid(account) && !only_at) {
    oidcd_handleUpdateIssuer(pipes, account_getIssuerUrl(account),
                             account_getName(account), INT_ACTION_VALUE_ADD);
    char* json = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, json);
    secFree(json);
    db_addAccountEncrypted(account);
    secFree(dce->device_code);
  } else if (only_at && strValid(account_getAccessToken(account))) {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS,
                    account_getAccessToken(account),
                    account_getIssuerUrl(account),
                    account_getTokenExpiresAt(account));
    secFreeDeviceCodeEntryContent(dce);
  } else {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "Could not get a refresh token");
    secFreeDeviceCodeEntryContent(dce);
  }
  deviceCodeDB_removeIfFound(dce);
}

void oidcd_handleStateLookUp(struct ipcPipe pipes, char* state) {
  agent_log(DEBUG, "Handle stateLookUp request");
  if (state == NULL || strlen(state) < 3) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "state malformed");
    return;
  }
  const unsigned char only_at = state[2] == '1' ? 1 : 0;
  struct oidc_account key     = {.usedState = state};
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
  if (only_at) {
    ipc_writeToPipe(pipes, RESPONSE_STATUS_ACCESS, STATUS_SUCCESS,
                    account_getAccessToken(account),
                    account_getIssuerUrl(account),
                    account_getTokenExpiresAt(account));
    accountDB_removeIfFound(account);
  } else {
    oidcd_handleUpdateIssuer(pipes, account_getIssuerUrl(account),
                             account_getName(account), INT_ACTION_VALUE_ADD);
    char* config = accountToJSONString(account);
    ipc_writeToPipe(pipes, RESPONSE_STATUS_CONFIG, STATUS_SUCCESS, config);
    secFree(config);
    db_addAccountEncrypted(account);  // reencrypting
  }
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

void oidcd_handleScopes(struct ipcPipe pipes, const char* issuer_url,
                        const char* config_endpoint, const char* cert_path) {
  if (issuer_url == NULL && config_endpoint == NULL) {
    ipc_writeToPipe(
        pipes, RESPONSE_ERROR,
        "Bad Request: issuer url or configuration endpoint must be given");
    return;
  }
  agent_log(DEBUG, "Handle scope lookup request for %s", issuer_url);
  char* scopes = getScopesSupportedFor(issuer_url, config_endpoint, cert_path);
  if (scopes == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, scopes);
  secFree(scopes);
}

void oidcd_handleMytokenProvidersLookup(struct ipcPipe pipes,
                                        const char*    mytoken_url,
                                        const char*    config_endpoint,
                                        const char*    cert_path) {
  if (mytoken_url == NULL && config_endpoint == NULL) {
    ipc_writeToPipe(
        pipes, RESPONSE_ERROR,
        "Bad Request: mytoken url or configuration endpoint must be given");
    return;
  }
  agent_log(DEBUG, "Handle mytoken OP lookup request for %s", mytoken_url);
  char* providers =
      getProvidersSupportedByMytoken(mytoken_url, config_endpoint, cert_path);
  if (providers == NULL) {
    ipc_writeOidcErrnoToPipe(pipes);
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO_OBJECT, providers);
  secFree(providers);
}

list_t* _getNameListLoadedAccounts() {
  list_t* accounts = accountDB_getList();
  list_t* names    = list_new();
  names->match     = (matchFunction)strequal;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(accounts, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* name = account_getName(node->val);
    list_rpush(names, list_node_new(name));
  }
  list_iterator_destroy(it);
  return names;
}

void oidcd_handleListLoadedAccounts(struct ipcPipe pipes) {
  list_t* names    = _getNameListLoadedAccounts();
  char*   jsonList = listToJSONArrayString(names);
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS_LOADEDACCOUNTS, jsonList);
  secFree(jsonList);
  secFreeList(names);
}

char* _argumentsToOptionsText(const struct arguments* arguments) {
  const char* const fmt      = "Lifetime:\t\t%s\n"
                               "Confirm:\t\t%s\n"
                               "Autoload:\t\t%s\n"
                               "Auto Re-authenticate:\t%s\n"
                               "Use custom URI scheme:\t%s\n"
                               "Webserver:\t\t%s\n"
                               "Allow ID-Token:\t\t%s\n"
                               "Group:\t\t\t%s\n"
                               "Daemon:\t\t\t%s\n"
                               "Log Debug:\t\t%s\n"
                               "Log to stderr:\t\t%s\n";
  char*             lifetime = arguments->lifetime
                                   ? oidc_sprintf("%lu seconds", arguments->lifetime)
                                   : oidc_strcopy("Forever");
  char*             options =
      oidc_sprintf(fmt, lifetime, arguments->confirm ? "true" : "false",
                   arguments->no_autoload ? "false" : "true",
                   arguments->no_autoreauthenticate ? "false" : "true",
                   arguments->no_scheme ? "false" : "true",
                   arguments->no_webserver ? "false" : "true",
                   arguments->always_allow_idtoken ? "true" : "false",
                   arguments->group ? arguments->group : "false",
                   arguments->console ? "false" : "true",
                   arguments->debug ? "true" : "false",
                   arguments->log_console ? "true" : "false");
  secFree(lifetime);
  return options;
}

char* _argumentsToCommandLineOptions(const struct arguments* arguments) {
  list_t* options = list_new();
  options->match  = (matchFunction)strequal;
  options->free   = (void (*)(void*))_secFree;

  if (arguments->lifetime) {
    list_rpush(options, list_node_new(oidc_sprintf("--lifetime=%ld",
                                                   arguments->lifetime)));
  }
  if (arguments->confirm) {
    list_rpush(options, list_node_new(oidc_strcopy("--confirm")));
  }
  if (arguments->no_autoload) {
    list_rpush(options, list_node_new(oidc_strcopy("--no-autoload")));
  }
  if (arguments->no_autoreauthenticate) {
    list_rpush(options, list_node_new(oidc_strcopy("--no-autoreauthenticate")));
  }
  if (arguments->no_scheme) {
    list_rpush(options, list_node_new(oidc_strcopy("--no-scheme")));
  }
  if (arguments->no_webserver) {
    list_rpush(options, list_node_new(oidc_strcopy("--no-webserver")));
  }
  if (arguments->always_allow_idtoken) {
    list_rpush(options, list_node_new(oidc_strcopy("--always-allow-idtoken")));
  }
  if (arguments->always_allow_idtoken) {
    list_rpush(options, list_node_new(oidc_strcopy("--always-allow-idtoken")));
  }
  if (arguments->group) {
    list_rpush(options,
               list_node_new(oidc_sprintf("--with-group", arguments->group)));
  }
  if (arguments->console) {
    list_rpush(options, list_node_new(oidc_strcopy("--console")));
  }
  if (arguments->debug) {
    list_rpush(options, list_node_new(oidc_strcopy("--debug")));
  }
  if (arguments->log_console) {
    list_rpush(options, list_node_new(oidc_strcopy("--log-stderr")));
  }
  char* opts = listToDelimitedString(options, " ");
  secFreeList(options);
  return opts;
}

void oidcd_handleAgentStatus(struct ipcPipe          pipes,
                             const struct arguments* arguments) {
  const char* fmt =
      "####################################\n"
      "##       oidc-agent status        ##\n"
      "####################################\n"
      "\nThis agent is running version %s.\n\nThis agent was started with the "
      "following options:\n%s\nCurrently there are %d accounts loaded: %s\n\n";
  list_t*      names      = _getNameListLoadedAccounts();
  unsigned int num_loaded = 0;
  char*        names_str  = NULL;
  if (names != NULL) {
    num_loaded = names->len;
    names_str  = listToDelimitedString(names, ", ");
  }
  char* options = _argumentsToOptionsText(arguments);
  char* status =
      oidc_sprintf(fmt, VERSION, options, num_loaded, names_str ?: "");
  secFree(options);
  secFree(names_str);
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO, status);
  secFreeList(names);
  secFree(status);
}

void oidcd_handleAgentStatusJSON(struct ipcPipe          pipes,
                                 const struct arguments* arguments) {
  list_t* names   = _getNameListLoadedAccounts();
  cJSON*  names_j = stringListToJSONArray(names);
  secFreeList(names);
  char*  options = _argumentsToCommandLineOptions(arguments);
  cJSON* json =
      generateJSONObject("version", cJSON_String, VERSION,
                         "command_line_options", cJSON_String, options, NULL);
  secFree(options);
  cJSON_AddItemToObject(json, "loaded_accounts",
                        names_j);  // names_j will freed with json
  char* info = jsonToString(json);
  secFreeJson(json);
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS_INFO_OBJECT, info);
  secFree(info);
}

void oidcd_handleFileWrite(struct ipcPipe pipes, const char* filename,
                           const char* data) {
  fileDB_addValue(filename, data);
  ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
}

void oidcd_handleFileRead(struct ipcPipe pipes, const char* filename) {
  char* data = fileDB_findValue(filename);
  if (data == NULL) {
    ipc_writeToPipe(pipes, RESPONSE_ERROR, "File not found");
    return;
  }
  ipc_writeToPipe(pipes, RESPONSE_SUCCESS_FILE, data);
  secFree(data);
}

void oidcd_handleFileRemove(struct ipcPipe pipes, const char* filename) {
  fileDB_removeIfFound(filename);
  ipc_writeToPipe(pipes, RESPONSE_STATUS_SUCCESS);
}
