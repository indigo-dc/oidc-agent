#include "access_token_handler.h"

#include "../ipc/ipc_values.h"
#include "../json.h"
#include "code.h"
#include "device.h"
#include "password.h"
#include "refresh.h"

#include "../../lib/list/src/list.h"

#include <syslog.h>

/** @fn oidc_error_t tryRefreshFlow(struct oidc_account* p)
 * @brief tries to issue an access token for the specified account by using the
 * refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
char* tryRefreshFlow(struct oidc_account* p, const char* scope) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Trying Refresh Flow");
  if (!strValid(account_getRefreshToken(*p))) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "No refresh token found");
    return NULL;
  }
  return refreshFlow(p, scope);
}

/** @fn oidc_error_t tryPasswordFlow(struct oidc_account* p)
 * @brief tries to issue an access token by using the password flow. The user
 * might be prompted for his username and password
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t tryPasswordFlow(struct oidc_account* p) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Trying Password Flow");
  if (!strValid(account_getUsername(*p)) ||
      !strValid(account_getPassword(*p))) {
    oidc_errno = OIDC_ECRED;
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No credentials found");
    return oidc_errno;
  }
  return passwordFlow(p);
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
int tokenIsValidForSeconds(struct oidc_account p, time_t min_valid_period) {
  time_t now        = time(NULL);
  time_t expires_at = account_getTokenExpiresAt(p);
  return expires_at - now > 0 && expires_at - now > min_valid_period;
}
char* getAccessTokenUsingRefreshFlow(struct oidc_account* account,
                                     time_t               min_valid_period,
                                     const char*          scope) {
  if (scope == NULL && min_valid_period != FORCE_NEW_TOKEN &&
      strValid(account_getAccessToken(*account)) &&
      tokenIsValidForSeconds(*account, min_valid_period)) {
    return account_getAccessToken(*account);
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "No acces token found that is valid long enough");
  return tryRefreshFlow(account, scope);
}

oidc_error_t getAccessTokenUsingPasswordFlow(struct oidc_account* account) {
  if (strValid(account_getAccessToken(*account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = tryPasswordFlow(account);
  return oidc_errno;
}

oidc_error_t getAccessTokenUsingAuthCodeFlow(struct oidc_account* account,
                                             const char*          code,
                                             const char* used_redirect_uri) {
  if (strValid(account_getAccessToken(*account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = codeExchange(account, code, used_redirect_uri);
  return oidc_errno;
}

oidc_error_t getAccessTokenUsingDeviceFlow(struct oidc_account* account,
                                           const char*          device_code) {
  if (strValid(account_getAccessToken(*account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = lookUpDeviceCode(account, device_code);
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
  flows->match  = (int (*)(void*, void*)) & strequal;
  if (flow == NULL) {  // Using default order
    list_rpush(flows, list_node_new(FLOW_VALUE_REFRESH));
    list_rpush(flows, list_node_new(FLOW_VALUE_PASSWORD));
    list_rpush(flows, list_node_new(FLOW_VALUE_CODE));
    list_rpush(flows, list_node_new(FLOW_VALUE_DEVICE));
    return flows;
  }
  flows->free = (void (*)(void*)) & secFree;
  if (flow[0] != '[') {
    list_rpush(flows, list_node_new(oidc_sprintf("%s", flow)));
    return flows;
  }
  list_destroy(flows);
  return JSONArrayStringToList(flow);
}
