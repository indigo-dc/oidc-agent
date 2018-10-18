#include "refresh.h"

#include "../account.h"
#include "../http/http.h"
#include "../utils/stringUtils.h"
#include "oidc.h"

#include <stddef.h>
#include <syslog.h>

char* generateRefreshPostData(struct oidc_account a, const char* scope) {
  char* refresh_token = account_getRefreshToken(a);
  char* client_id     = account_getClientId(a);
  char* client_secret = account_getClientSecret(a);
  char* str =
      strValid(scope)
          ? generatePostData("client_id", client_id, "client_secret",
                             client_secret, "grant_type", "refresh_token",
                             "refresh_token", refresh_token, "scope", scope,
                             NULL)
          : generatePostData("client_id", client_id, "client_secret",
                             client_secret, "grant_type", "refresh_token",
                             "refresh_token", refresh_token, NULL);
  secFree(refresh_token);
  secFree(client_id);
  secFree(client_secret);
  return str;
}

/** @fn oidc_error_t refreshFlow(struct oidc_account* p)
 * @brief issues an access token via refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
char* refreshFlow(struct oidc_account* p, const char* scope) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing RefreshFlow\n");
  char* data = generateRefreshPostData(*p, scope);
  if (data == NULL) {
    return NULL;
    ;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* client_id     = account_getClientId(*p);
  char* client_secret = account_getClientSecret(*p);
  char* res = sendPostDataWithBasicAuth(account_getTokenEndpoint(*p), data,
                                        account_getCertPath(*p), client_id,
                                        client_secret);
  secFree(client_id);
  secFree(client_secret);
  secFree(data);
  if (NULL == res) {
    return NULL;
    ;
  }

  char* access_token = parseTokenResponse(res, p, !strValid(scope), 0);
  secFree(res);
  return access_token;
}
