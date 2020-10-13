#include "access_token_handler.h"
#include "code.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "device.h"
#include "list/list.h"
#include "oidc-agent/oidc/flows/oidc.h"
#include "password.h"
#include "refresh.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/stringUtils.h"

char* tryRefreshFlow(struct oidc_account* p, const char* scope,
                     const char* audience, struct ipcPipe pipes) {
  agent_log(DEBUG, "Trying Refresh Flow");
  if (!account_refreshTokenIsValid(p)) {
    agent_log(ERROR, "No refresh token found");
    oidc_errno = OIDC_ENOREFRSH;
    return NULL;
  }
  return refreshFlow(TOKENPARSEMODE_RETURN_AT, p, scope, audience, pipes);
}

char* getIdToken(struct oidc_account* p, const char* scope,
                 struct ipcPipe pipes) {
  if (!account_refreshTokenIsValid(p)) {
    agent_log(ERROR, "No refresh token found");
    oidc_errno = OIDC_ENOREFRSH;
    return NULL;
  }
  return refreshFlow(TOKENPARSEMODE_RETURN_ID, p, scope, NULL, pipes);
}

/** @fn oidc_error_t tryPasswordFlow(struct oidc_account* p)
 * @brief tries to issue an access token by using the password flow. The user
 * might be prompted for his username and password
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t tryPasswordFlow(struct oidc_account* p, struct ipcPipe pipes,
                             const char* scope) {
  agent_log(DEBUG, "Trying Password Flow");
  if (!strValid(account_getUsername(p)) || !strValid(account_getPassword(p))) {
    oidc_errno = OIDC_ECRED;
    agent_log(DEBUG, "No credentials found");
    return oidc_errno;
  }
  return passwordFlow(p, pipes, scope);
}

/** @fn int tokenIsValidforSeconds(struct oidc_account p, time_t
 * min_valid_period)
 * @brief checks if the access token for an account is at least valid for the
 * given period of time
 * @param p the account whose access token should be checked
 * @param min_valid_period the period of time the access token should be valid
 * (at least)
 * @return 1 if the access_token is valid for the given time; 0 if not.
 */
int tokenIsValidForSeconds(const struct oidc_account* p,
                           time_t                     min_valid_period) {
  time_t now        = time(NULL);
  time_t expires_at = account_getTokenExpiresAt(p);
  return expires_at - now > 0 && expires_at - now > min_valid_period;
}

char* getAccessTokenUsingRefreshFlow(struct oidc_account* account,
                                     time_t min_valid_period, const char* scope,
                                     const char*    audience,
                                     struct ipcPipe pipes) {
  if (scope == NULL && audience == NULL &&
      min_valid_period != FORCE_NEW_TOKEN &&
      strValid(account_getAccessToken(account)) &&
      tokenIsValidForSeconds(account, min_valid_period)) {
    return account_getAccessToken(account);
  }
  agent_log(DEBUG, "No access token found that is valid long enough");
  return tryRefreshFlow(account, scope, audience, pipes);
}

oidc_error_t getAccessTokenUsingPasswordFlow(struct oidc_account* account,
                                             struct ipcPipe       pipes,
                                             const char*          scope) {
  if (scope == NULL && strValid(account_getAccessToken(account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = tryPasswordFlow(account, pipes, scope);
  return oidc_errno;
}

oidc_error_t getAccessTokenUsingAuthCodeFlow(struct oidc_account* account,
                                             const char*          code,
                                             const char*    used_redirect_uri,
                                             char*          code_verifier,
                                             struct ipcPipe pipes) {
  if (strValid(account_getAccessToken(account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno =
      codeExchange(account, code, used_redirect_uri, code_verifier, pipes);
  return oidc_errno;
}

oidc_error_t getAccessTokenUsingDeviceFlow(struct oidc_account* account,
                                           const char*          device_code,
                                           struct ipcPipe       pipes) {
  if (strValid(account_getAccessToken(account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = lookUpDeviceCode(account, device_code, pipes);
  return oidc_errno;
}

struct flow_order {
  unsigned char refresh;
  unsigned char password;
  unsigned char code;
  unsigned char device;
};

list_t* parseFlow(const char* flow) {
  list_t* flows = list_new();
  flows->match  = (matchFunction)strequal;
  if (flow == NULL) {  // Using default order
    list_rpush(flows, list_node_new(FLOW_VALUE_REFRESH));
    list_rpush(flows, list_node_new(FLOW_VALUE_PASSWORD));
    list_rpush(flows, list_node_new(FLOW_VALUE_CODE));
    list_rpush(flows, list_node_new(FLOW_VALUE_DEVICE));
    return flows;
  }
  flows->free = (void (*)(void*)) & _secFree;
  if (flow[0] != '[') {
    list_rpush(flows, list_node_new(oidc_sprintf("%s", flow)));
    return flows;
  }
  list_destroy(flows);
  return JSONArrayStringToList(flow);
}
