#include "revoke.h"

#include "account/account.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "oidc-agent/oidc/values.h"
#include "oidc.h"

#include <syslog.h>

oidc_error_t revokeToken(struct oidc_account* account) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Performing Token revocation flow");
  if (!strValid(account_getRevocationEndpoint(account))) {
    oidc_errno = OIDC_ENOSUPREV;
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "%s", oidc_serror());
    return oidc_errno;
  }
  char* refresh_token = account_getRefreshToken(account);
  char* data = generatePostData(OIDC_KEY_TOKENTYPE_HINT, OIDC_TOKENTYPE_REFRESH,
                                OIDC_KEY_TOKEN, refresh_token, NULL);
  if (data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(account_getRevocationEndpoint(account),
                                        data, account_getCertPath(account),
                                        account_getClientId(account),
                                        account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    if (oidc_errno == OIDC_EHTTP0) {
      account_setRefreshToken(account, NULL);
      oidc_errno = OIDC_SUCCESS;
      syslog(
          LOG_AUTHPRIV | LOG_INFO,
          "Ignored http0 error - empty response allowed for token revocation");
    }
    return oidc_errno;
  }
  char* error = parseForError(res);
  if (error) {
    oidc_errno = OIDC_EOIDC;
    oidc_seterror(error);
    secFree(error);
    return oidc_errno;
  }
  account_setRefreshToken(account, NULL);
  oidc_errno = OIDC_SUCCESS;
  return oidc_errno;
}
