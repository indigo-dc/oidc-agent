#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "oidc-gen/gen_handler.h"

void askOrNeedScope(struct oidc_account*    account,
                    const struct arguments* arguments, int optional) {
  // TODO if --pub set, should use the max of this public client
  char* supportedScope =
      compIssuerUrls(account_getIssuerUrl(account), ELIXIR_ISSUER_URL)
          ? oidc_strcopy(ELIXIR_SUPPORTED_SCOPES)
          : gen_handleScopeLookup(account_getIssuerUrl(account),
                                  account_getCertPath(account));
  if (readScope(account, arguments)) {
    if (strequal(account_getScope(account), AGENT_SCOPE_ALL)) {
      account_setScope(account, supportedScope);
    }
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("scope", OPT_LONG_SCOPE));
  printNormal("This issuer supports the following scopes: %s\n",
              supportedScope);
  if (!strValid(account_getScope(account))) {
    account_setScope(account, oidc_strcopy(DEFAULT_SCOPE));
  }
  char* res = _gen_promptMultipleSpaceSeparated(
      "Scopes or 'max'", account_getScope(account), optional);
  if (res) {
    account_setScopeExact(account, res);
  }
  if (strequal(account_getScope(account), AGENT_SCOPE_ALL)) {
    account_setScope(account, supportedScope);
  } else {
    secFree(supportedScope);
  }
}

int readScope(struct oidc_account* account, const struct arguments* arguments) {
  if (arguments->scope) {
    account_setScope(account, oidc_strcopy(arguments->scope));
    return 1;
  }
  return 0;
}

void askScope(struct oidc_account* account, const struct arguments* arguments) {
  return askOrNeedScope(account, arguments, 1);
}

void needScope(struct oidc_account*    account,
               const struct arguments* arguments) {
  return askOrNeedScope(account, arguments, 0);
}
