#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/stringUtils.h"

void askOrNeedUsername(struct oidc_account*    account,
                       const struct arguments* arguments, int optional) {
  if (readUsername(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("username", OPT_LONG_USERNAME));
  char* res =
      _gen_prompt("Username", account_getUsername(account), 0, optional);
  if (res) {
    account_setUsername(account, res);
  }
}

int readUsername(struct oidc_account*    account,
                 const struct arguments* arguments) {
  if (arguments->op_username) {
    account_setUsername(account, oidc_strcopy(arguments->op_username));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getUsername(account))) {
    return 1;
  }
  return 0;
}

void askUsername(struct oidc_account*    account,
                 const struct arguments* arguments) {
  return askOrNeedUsername(account, arguments, 1);
}

void needUsername(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedUsername(account, arguments, 0);
}
