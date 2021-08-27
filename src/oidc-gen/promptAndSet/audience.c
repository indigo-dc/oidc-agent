#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/string/stringUtils.h"

void askOrNeedAudience(struct oidc_account*    account,
                       const struct arguments* arguments, int optional) {
  if (readAudience(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("audience", OPT_LONG_AUDIENCE));
  char* res = _gen_promptMultipleSpaceSeparated(
      "Audiences", account_getAudience(account), optional);
  if (res) {
    account_setAudience(account, res);
  }
}

int readAudience(struct oidc_account*    account,
                 const struct arguments* arguments) {
  if (arguments->audience) {
    account_setAudience(account, oidc_strcopy(arguments->audience));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getAudience(account))) {
    return 1;
  }
  return 0;
}

void askAudience(struct oidc_account*    account,
                 const struct arguments* arguments) {
  return askOrNeedAudience(account, arguments, 1);
}

void needAudience(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedAudience(account, arguments, 0);
}
