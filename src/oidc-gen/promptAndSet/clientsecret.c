#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/string/stringUtils.h"

void askOrNeedClientSecret(struct oidc_account*    account,
                           const struct arguments* arguments, int optional) {
  if (readClientSecret(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional,
                     ERROR_MESSAGE("client secret", OPT_LONG_CLIENTSECRET));
  char* res = _gen_prompt("Client_secret", account_getClientSecret(account), 1,
                          optional);
  if (res) {
    account_setClientSecret(account, res);
  }
}

int readClientSecret(struct oidc_account*    account,
                     const struct arguments* arguments) {
  if (arguments->client_secret) {
    account_setClientSecret(account, oidc_strcopy(arguments->client_secret));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getClientSecret(account))) {
    return 1;
  }
  return 0;
}

void askClientSecret(struct oidc_account*    account,
                     const struct arguments* arguments) {
  return askOrNeedClientSecret(account, arguments, 1);
}

void needClientSecret(struct oidc_account*    account,
                      const struct arguments* arguments) {
  return askOrNeedClientSecret(account, arguments, 0);
}
