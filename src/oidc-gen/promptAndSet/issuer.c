#include "_helper.h"
#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/msys.h"
#include "promptAndSet.h"
#include "utils/config/issuerConfig.h"
#include "utils/listUtils.h"
#include "utils/prompt.h"
#include "utils/string/stringUtils.h"

void _suggestTheseIssuers(list_t* issuers, struct oidc_account* account,
                          int optional) {
  size_t favPos = getFavIssuer(account, issuers);
  char*  iss = promptSelect("Please select issuer", "Issuer", issuers, favPos,
                            CLI_PROMPT_NOT_VERBOSE);
  if (!strValid(iss)) {
    printError("Something went wrong. Invalid Issuer.\n");
    if (optional) {
      return;
    }
    exit(EXIT_FAILURE);
  }
  account_setIssuerUrl(account, iss);
}

void askOrNeedIssuer(struct oidc_account*    account,
                     const struct arguments* arguments, int optional) {
  if (readIssuer(account, arguments) ||
      strValid(account_getIssuerUrl(account))) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("issuer url", OPT_LONG_ISSUER));
  list_t* issuers = getSuggestableIssuers();
  if (listValid(issuers)) {
    char* res =
        _gen_prompt("Issuer", account_getIssuerUrl(account), 0, optional);
    if (res) {
      account_setIssuerUrl(account, res);
    }
  } else {
    _suggestTheseIssuers(issuers, account, optional);
  }
  secFreeList(issuers);
}

int readIssuer(struct oidc_account*    account,
               const struct arguments* arguments) {
  if (arguments->issuer) {
    account_setIssuerUrl(account, oidc_strcopy(arguments->issuer));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getIssuerUrl(account))) {
    return 1;
  }
  return strValid(account_getConfigEndpoint(account));
}

void askIssuer(struct oidc_account*    account,
               const struct arguments* arguments) {
  return askOrNeedIssuer(account, arguments, 1);
}

void needIssuer(struct oidc_account*    account,
                const struct arguments* arguments) {
  return askOrNeedIssuer(account, arguments, 0);
}
