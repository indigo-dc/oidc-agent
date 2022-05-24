#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/string/stringUtils.h"

void askOrNeedMyProfile(struct oidc_account*    account,
                        const struct arguments* arguments, int optional) {
  if (readMyProfile(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional,
                     ERROR_MESSAGE("mytoken profile", OPT_LONG_MYTOKENPROFILE));
  char* res = _gen_prompt("Mytoken Profile",
                          account_getUsedMytokenProfile(account), 0, optional);
  if (res) {
    account_setUsedMytokenProfile(account, res);
  }
}

int readMyProfile(struct oidc_account*    account,
                  const struct arguments* arguments) {
  if (arguments->mytoken_profile) {
    account_setUsedMytokenProfile(account,
                                  oidc_strcopy(arguments->mytoken_profile));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getUsedMytokenProfile(account))) {
    return 1;
  }
  return 0;
}

void askMyProfile(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedMyProfile(account, arguments, 1);
}

void needMyProfile(struct oidc_account*    account,
                   const struct arguments* arguments) {
  return askOrNeedMyProfile(account, arguments, 0);
}
