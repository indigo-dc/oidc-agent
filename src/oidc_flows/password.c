#include "password.h"

#include "oidc.h"
#include "../account.h"
#include "../http/http.h"
#include "../oidc_error.h"
#include "../utils/stringUtils.h"

#include <stddef.h>
#include <syslog.h>

char* generatePasswordPostData(struct oidc_account a) {
  return generatePostData(
      "client_id", account_getClientId(a),
      "client_secret", account_getClientSecret(a),
      "grant_type", "password", 
      "username", account_getUsername(a),
      "password", account_getPassword(a),
      NULL);
}

/** @fn oidc_error_t passwordFlow(struct oidc_account* p)
 * @brief issues an access token using the password flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t passwordFlow(struct oidc_account* p) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing RefreshFlow\n");
  char* data = generatePasswordPostData(*p);
  if(data == NULL) {
    return oidc_errno;;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = sendPostDataWithBasicAuth(account_getTokenEndpoint(*p), data, account_getCertPath(*p), account_getClientId(*p), account_getClientSecret(*p));
  clearFreeString(data);
  if(NULL==res) {
    return oidc_errno;;
  }

  char* access_token = parseTokenResponse(res, p, 1, 1);
  clearFreeString(res);
  return access_token==NULL ? oidc_errno : OIDC_SUCCESS;
}
