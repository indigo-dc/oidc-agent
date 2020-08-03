#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "utils/stringUtils.h"

void askOrNeedDeviceAuthEndpoint(struct oidc_account*    account,
                                 const struct arguments* arguments,
                                 int                     optional) {
  if (readDeviceAuthEndpoint(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("device authorization endpoint",
                                             OPT_LONG_DEVICE));
  char* res =
      _gen_prompt("Device Authorization Endpoint",
                  account_getDeviceAuthorizationEndpoint(account), 0, optional);
  if (res) {
    issuer_setDeviceAuthorizationEndpoint(account_getIssuer(account), res, 1);
  }
}

int readDeviceAuthEndpoint(struct oidc_account*    account,
                           const struct arguments* arguments) {
  if (arguments->device_authorization_endpoint) {
    issuer_setDeviceAuthorizationEndpoint(
        account_getIssuer(account),
        oidc_strcopy(arguments->device_authorization_endpoint), 1);
    return 1;
  }
  return 0;
}

void askDeviceAuthEndpoint(struct oidc_account*    account,
                           const struct arguments* arguments) {
  return askOrNeedDeviceAuthEndpoint(account, arguments, 1);
}

void needDeviceAuthEndpoint(struct oidc_account*    account,
                            const struct arguments* arguments) {
  return askOrNeedDeviceAuthEndpoint(account, arguments, 0);
}
