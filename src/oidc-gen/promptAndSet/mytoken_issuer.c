#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/listUtils.h"
#include "utils/prompt.h"
#include "utils/string/stringUtils.h"

void askOrNeedMytokenIssuer(struct oidc_account*    account,
                            const struct arguments* arguments, int optional) {
  if (readMytokenIssuer(account, arguments) ||
      strValid(account_getMytokenUrl(account))) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional,
                     ERROR_MESSAGE("mytoken url", OPT_LONG_MYTOKENURL));
  char* res =
      _gen_prompt("Mytoken URL", account_getMytokenUrl(account), 0, optional);
  if (res) {
    account_setMytokenUrl(account, res);
  }
}

int readMytokenIssuer(struct oidc_account*    account,
                      const struct arguments* arguments) {
  if (arguments->mytoken_issuer.str) {
    account_setMytokenUrl(account, oidc_strcopy(arguments->mytoken_issuer.str));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getMytokenUrl(account))) {
    return 1;
  }
  return strValid(account_getConfigEndpoint(account));
}

void askMytokenIssuer(struct oidc_account*    account,
                      const struct arguments* arguments) {
  return askOrNeedMytokenIssuer(account, arguments, 1);
}

void needMytokenIssuer(struct oidc_account*    account,
                       const struct arguments* arguments) {
  return askOrNeedMytokenIssuer(account, arguments, 0);
}
