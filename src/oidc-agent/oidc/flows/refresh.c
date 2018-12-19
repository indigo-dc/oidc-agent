#include "refresh.h"

#include "account/account.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/stringUtils.h"

#include <stddef.h>
#include <syslog.h>

char* generateRefreshPostData(const struct oidc_account* a, const char* scope) {
  char*       refresh_token = account_getRefreshToken(a);
  const char* useThisScope =
      strValid(scope)
          ? scope
          : account_getScope(a);  // if scopes are explicilty set use these, if
                                  // not we use the same as for the used refresh
                                  // token. Usually this parameter can be
                                  // omitted. For unity we have to include this.
  char* str =
      strValid(useThisScope)
          ? generatePostData("client_id", account_getClientId(a),
                             "client_secret", account_getClientSecret(a),
                             "grant_type", "refresh_token", "refresh_token",
                             refresh_token, "scope", useThisScope, NULL)
          : generatePostData("client_id", account_getClientId(a),
                             "client_secret", account_getClientSecret(a),
                             "grant_type", "refresh_token", "refresh_token",
                             refresh_token, NULL);
  return str;
}

/** @fn oidc_error_t refreshFlow(struct oidc_account* p)
 * @brief issues an access token via refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
char* refreshFlow(struct oidc_account* p, const char* scope) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing RefreshFlow\n");
  char* data = generateRefreshPostData(p, scope);
  if (data == NULL) {
    return NULL;
    ;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(p), data, account_getCertPath(p),
      account_getClientId(p), account_getClientSecret(p));
  secFree(data);
  if (NULL == res) {
    return NULL;
    ;
  }

  char* access_token = parseTokenResponse(res, p, !strValid(scope), 0);
  secFree(res);
  return access_token;
}
