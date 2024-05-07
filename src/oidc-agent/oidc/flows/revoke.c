#include "revoke.h"

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/config/custom_parameter.h"
#include "utils/parseJson.h"
#include "utils/string/stringUtils.h"

oidc_error_t _revokeToken(struct oidc_account* account,
                          unsigned char        withClientId) {
  agent_log(DEBUG, "Performing Token revocation flow");
  if (!strValid(account_getRevocationEndpoint(account))) {
    oidc_errno = OIDC_ENOSUPREV;
    agent_log(NOTICE, "%s", oidc_serror());
    return oidc_errno;
  }
  char* refresh_token = account_getRefreshToken(account);
  char* data          = generatePostData(
      OIDC_REQUEST_TYPE_REVOKE, account, OIDC_KEY_TOKENTYPE_HINT,
      OIDC_TOKENTYPE_REFRESH, OIDC_KEY_TOKEN, refresh_token,
      withClientId ? OIDC_KEY_CLIENTID : NULL,
      withClientId ? account_getClientId(account) : NULL, NULL);
  if (data == NULL) {
    return oidc_errno;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* cert_path = account_getCertPathOrDefault(account);
  char* res       = sendPostDataWithBasicAuth(
      account_getRevocationEndpoint(account), data, cert_path,
      account_getClientId(account), account_getClientSecret(account));
  secFree(cert_path);
  secFree(data);
  if (res == NULL) {
    if (oidc_errno == OIDC_EHTTP0) {
      account_setRefreshToken(account, NULL);
      oidc_errno = OIDC_SUCCESS;
      agent_log(
          INFO,
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

oidc_error_t revokeToken(struct oidc_account* account) {
  oidc_error_t err = _revokeToken(account, 0);
  if (err == OIDC_SUCCESS) {
    return err;
  }
  struct oidc_error_state* errorState = saveErrorState();
  err                                 = _revokeToken(account, 1);
  if (err == OIDC_SUCCESS) {
    secFreeErrorState(errorState);
    return err;
  }
  restoreAndFreeErrorState(errorState);
  return oidc_errno;
}
