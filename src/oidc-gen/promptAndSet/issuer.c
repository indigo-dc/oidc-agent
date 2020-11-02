#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/prompt.h"
#include "utils/stringUtils.h"

void _useSuggestedIssuer(struct oidc_account* account, int optional) {
  list_t* issuers = getSuggestableIssuers();
  size_t  favPos  = getFavIssuer(account, issuers);
  char*   iss = promptSelect("Please select issuer", "Issuer", issuers, favPos,
                           CLI_PROMPT_NOT_VERBOSE);
  secFreeList(issuers);
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
  if (readIssuer(account, arguments)) {
    stringifyIssuerUrl(account);
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("issuer url", OPT_LONG_ISSUER));
  if (!oidcFileDoesExist(ISSUER_CONFIG_FILENAME) &&
      !fileDoesExist(ETC_ISSUER_CONFIG_FILE)) {
    char* res =
        _gen_prompt("Issuer", account_getIssuerUrl(account), 0, optional);
    if (res) {
      account_setIssuerUrl(account, res);
    }
  } else {
    _useSuggestedIssuer(account, optional);
  }
  stringifyIssuerUrl(account);
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
  return 0;
}

void askIssuer(struct oidc_account*    account,
               const struct arguments* arguments) {
  return askOrNeedIssuer(account, arguments, 1);
}

void needIssuer(struct oidc_account*    account,
                const struct arguments* arguments) {
  return askOrNeedIssuer(account, arguments, 0);
}
