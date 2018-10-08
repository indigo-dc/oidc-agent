#include "revoke.h"

#include "../account.h"
#include "../http/http.h"
#include "../json.h"
#include "../parse_oidp.h"
#include "oidc.h"

#include <syslog.h>

oidc_error_t revokeToken(struct oidc_account* account) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Performing Token revocation flow");
  if (!strValid(account_getRevocationEndpoint(*account))) {
    oidc_seterror("Token revocation is not supported by this issuer.");
    oidc_errno = OIDC_EERROR;
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "%s", oidc_serror());
    return oidc_errno;
  }
  char* data = generatePostData("token_type_hint", "refresh_token", "token",
                                account_getRefreshToken(*account), NULL);
  if (data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(account_getRevocationEndpoint(*account),
                                        data, account_getCertPath(*account),
                                        account_getClientId(*account),
                                        account_getClientSecret(*account));
  clearFreeString(data);
  if (strValid(res) && parseForError(res) == NULL) {
    account_setRefreshToken(account, NULL);
    oidc_errno = OIDC_SUCCESS;
  }
  clearFreeString(res);
  return oidc_errno;
}
