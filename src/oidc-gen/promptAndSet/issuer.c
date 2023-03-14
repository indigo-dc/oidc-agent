#include "_helper.h"
#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/msys.h"
#include "promptAndSet.h"
#include "utils/config/issuerConfig.h"
#include "utils/listUtils.h"
#include "utils/prompt.h"
#include "utils/string/stringUtils.h"

static void applyIssuerDefaultConfig(struct oidc_account*    account,
                                     const struct arguments* arguments) {
  const struct issuerConfig* iss =
      getIssuerConfig(account_getIssuerUrl(account));
  if (iss==NULL) {
    return;
  }
  if (arguments->configuration_endpoint == NULL &&
      iss->configuration_endpoint != NULL) {
    account_setConfigEndpoint(account,
                              oidc_strcopy(iss->configuration_endpoint));
  }
  if (arguments->device_authorization_endpoint == NULL &&
      iss->device_authorization_endpoint != NULL) {
    issuer_setDeviceAuthorizationEndpoint(
        account_getIssuer(account),
        oidc_strcopy(iss->device_authorization_endpoint), 1);
  }
  if (arguments->cert_path == NULL && iss->cert_path != NULL) {
    account_setCertPath(account, oidc_strcopy(iss->cert_path));
  }
  if (!arguments->oauth && iss->oauth) {
    account_setOAuth2(account);
  }
}

void _suggestTheseIssuers(list_t* issuers, struct oidc_account* account,
                          const struct arguments* arguments, int optional) {
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
  applyIssuerDefaultConfig(account, arguments);
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
    _suggestTheseIssuers(issuers, account, arguments, optional);
  } else {
    char* res =
        _gen_prompt("Issuer", account_getIssuerUrl(account), 0, optional);
    if (res) {
      account_setIssuerUrl(account, res);
      applyIssuerDefaultConfig(account, arguments);
    }
  }
  secFreeList(issuers);
}

int readIssuer(struct oidc_account*    account,
               const struct arguments* arguments) {
  if (arguments->issuer) {
    account_setIssuerUrl(account, oidc_strcopy(arguments->issuer));
    applyIssuerDefaultConfig(account, arguments);
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
