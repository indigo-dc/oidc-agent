#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/string/stringUtils.h"

void askOrNeedConfigEndpoint(struct oidc_account*    account,
                             const struct arguments* arguments, int optional) {
  if (readConfigEndpoint(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("configuration endpoint",
                                             OPT_LONG_CONFIG_ENDPOINT));
  char* res = _gen_prompt("Configuration Endpoint",
                          account_getConfigEndpoint(account), 0, optional);
  if (res) {
    account_setConfigEndpoint(account, res);
  }
}

int readConfigEndpoint(struct oidc_account*    account,
                       const struct arguments* arguments) {
  if (arguments->configuration_endpoint) {
    account_setConfigEndpoint(account,
                              oidc_strcopy(arguments->configuration_endpoint));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getConfigEndpoint(account))) {
    return 1;
  }
  return 0;
}

void askConfigEndpoint(struct oidc_account*    account,
                       const struct arguments* arguments) {
  return askOrNeedConfigEndpoint(account, arguments, 1);
}

void needConfigEndpoint(struct oidc_account*    account,
                        const struct arguments* arguments) {
  return askOrNeedConfigEndpoint(account, arguments, 0);
}
