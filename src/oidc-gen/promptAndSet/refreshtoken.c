#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "utils/stringUtils.h"

void askOrNeedRefreshToken(struct oidc_account*    account,
                           const struct arguments* arguments, int optional) {
  if (readRefreshToken(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional,
                     ERROR_MESSAGE("refresh token", OPT_LONG_REFRESHTOKEN));
  char* res = _gen_prompt("Refresh token", account_getRefreshToken(account), 0,
                          optional);
  if (res) {
    account_setRefreshToken(account, res);
  }
}

int readRefreshToken(struct oidc_account*    account,
                     const struct arguments* arguments) {
  if (arguments->refresh_token) {
    account_setRefreshToken(account, oidc_strcopy(arguments->refresh_token));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getRefreshToken(account))) {
    return 1;
  }
  return 0;
}

void askRefreshToken(struct oidc_account*    account,
                     const struct arguments* arguments) {
  return askOrNeedRefreshToken(account, arguments, 1);
}

void needRefreshToken(struct oidc_account*    account,
                      const struct arguments* arguments) {
  return askOrNeedRefreshToken(account, arguments, 0);
}
