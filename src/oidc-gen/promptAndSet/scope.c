#include "_helper.h"
#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "oidc-gen/gen_handler.h"
#include "promptAndSet.h"
#include "utils/string/stringUtils.h"

char* getSupportedScopes(struct oidc_account*    account,
                         const struct arguments* arguments) {
  if (arguments->usePublicClient) {
    char* pubScopes = getScopesForPublicClient(account);
    if (strValid(pubScopes)) {
      return pubScopes;
    }
  }
  if (compIssuerUrls(account_getIssuerUrl(account), ELIXIR_ISSUER_URL)) {
    return oidc_strcopy(ELIXIR_SUPPORTED_SCOPES);
  }
  return gen_handleScopeLookup(account);
}

void askOrNeedScope(struct oidc_account*    account,
                    const struct arguments* arguments, int optional) {
  _askOrNeedScope(getSupportedScopes(account, arguments), account, arguments,
                  optional);
}
void _askOrNeedScope(char* supportedScope, struct oidc_account* account,
                     const struct arguments* arguments, int optional) {
  if (readScope(account, arguments)) {
    if (strequal(account_getScope(account), AGENT_SCOPE_ALL)) {
      account_setScope(account, supportedScope);
    }
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("scope", OPT_LONG_SCOPE));
  printNormal("The following scopes are supported: %s\n", supportedScope);
  if (!strValid(account_getScope(account))) {
    account_setScope(account, oidc_strcopy(DEFAULT_SCOPE));
  }
  char* res = _gen_promptMultipleSpaceSeparated(
      "Scopes or '" AGENT_SCOPE_ALL "'", account_getScope(account), optional);
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
    void (*setter)(struct oidc_account*, char*) = account_setScope;
    if (strequal(arguments->scope, AGENT_SCOPE_ALL)) {
      setter = account_setScopeExact;
    }
    setter(account, oidc_strcopy(arguments->scope));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getScope(account))) {
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
