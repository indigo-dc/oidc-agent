#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/prompt.h"
#include "utils/stringUtils.h"

void askOrNeedName(struct oidc_account*    account,
                   const struct arguments* arguments, int optional) {
  if (readName(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, "No account short name given.");
  char* shortname = NULL;
  do {
    secFree(shortname);
    shortname = prompt("Enter short name for the account to configure",
                       "short name", NULL, CLI_PROMPT_VERBOSE);
  } while (!strValid(shortname) || optional);
  if (shortname) {
    account_setName(account, shortname, arguments->cnid);
  }
}

int readName(struct oidc_account* account, const struct arguments* arguments) {
  if (arguments->args[0]) {
    account_setName(account, oidc_strcopy(arguments->args[0]), arguments->cnid);
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getName(account))) {
    return 1;
  }
  return 0;
}

void askName(struct oidc_account* account, const struct arguments* arguments) {
  return askOrNeedName(account, arguments, 1);
}

void needName(struct oidc_account* account, const struct arguments* arguments) {
  return askOrNeedName(account, arguments, 0);
}
