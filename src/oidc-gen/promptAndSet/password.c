#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "utils/stringUtils.h"

void askOrNeedPassword(struct oidc_account*    account,
                       const struct arguments* arguments, int optional) {
  if (readPassword(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("password", OPT_LONG_PASSWORD));
  char* res =
      _gen_prompt("Password", account_getPassword(account), 1, optional);
  if (res) {
    account_setPassword(account, res);
  }
}

int readPassword(struct oidc_account*    account,
                 const struct arguments* arguments) {
  if (arguments->op_password) {
    account_setPassword(account, oidc_strcopy(arguments->op_password));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getPassword(account))) {
    return 1;
  }
  return 0;
}

void askPassword(struct oidc_account*    account,
                 const struct arguments* arguments) {
  return askOrNeedPassword(account, arguments, 1);
}

void needPassword(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedPassword(account, arguments, 0);
}
