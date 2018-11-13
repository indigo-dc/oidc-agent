#include "token_exchange.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"

#include <syslog.h>

char* generateTokenExchangePostData(struct oidc_account a) {
  return generatePostData(
      "grant_type", "urn:ietf:params:oauth:grant-type:token-exchange",
      "subject_token", account_getAccessToken(a), "subject_token_type",
      "urn:ietf:params:oauth:token-type:access_token", "requested_token_type",
      "urn:ietf:params:oauth:token-type:refresh_token",
      // "scope",account_getScope(a),
      NULL);
}

/**
 * @brief issues an access token using token exchange
 * @param account a pointer to the account for whom an access token should be
 * issued; it must include the access token that should be exchanged and the
 * client credentials for a client that has the token exchange grant.
 * @return 0 on success; an oidc_error code otherwise
 */
oidc_error_t tokenExchange(struct oidc_account* account) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "performing token exchange");
  char* data = generateTokenExchangePostData(*account);
  if (data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(*account), data, account_getCertPath(*account),
      account_getClientId(*account), account_getClientSecret(*account));
  secFree(data);
  if (NULL == res) {
    return oidc_errno;
  }
  char* access_token = parseTokenResponse(res, account, 1, 1);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}
