#include "password.h"

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <stddef.h>

char* generatePasswordPostData(const struct oidc_account* a) {
  return generatePostData(
      // OIDC_KEY_CLIENTID, account_getClientId(a),
      // OIDC_KEY_CLIENTSECRET, account_getClientSecret(a),
      OIDC_KEY_GRANTTYPE, OIDC_GRANTTYPE_PASSWORD, OIDC_KEY_USERNAME,
      account_getUsername(a), OIDC_KEY_PASSWORD, account_getPassword(a), NULL);
}

/** @fn oidc_error_t passwordFlow(struct oidc_account* p)
 * @brief issues an access token using the password flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t passwordFlow(struct oidc_account* p, struct ipcPipe pipes) {
  agent_log(DEBUG, "Doing PasswordFlow\n");
  char* data = generatePasswordPostData(p);
  if (data == NULL) {
    return oidc_errno;
    ;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(p), data, account_getCertPath(p),
      account_getClientId(p), account_getClientSecret(p));
  secFree(data);
  if (NULL == res) {
    return oidc_errno;
    ;
  }

  char* access_token = parseTokenResponse(res, p, 1, pipes);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}
