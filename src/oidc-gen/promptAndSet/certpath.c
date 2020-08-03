#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "utils/stringUtils.h"

void askOrNeedCertPath(struct oidc_account*    account,
                       const struct arguments* arguments, int optional) {
  if (readCertPath(account, arguments)) {
    return;
  }
  if (strValid(account_getCertPath(account))) {
    optional = 1;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("cert path", OPT_LONG_CERTPATH));
  char* res =
      _gen_prompt("Cert Path", account_getCertPath(account), 0, optional);
  if (res) {
    account_setCertPath(account, res);
  }
}

int readCertPath(struct oidc_account*    account,
                 const struct arguments* arguments) {
  if (arguments->cert_path) {
    account_setCertPath(account, oidc_strcopy(arguments->cert_path));
    return 1;
  }
  account_setOSDefaultCertPath(account);
  return 0;
}

void askCertPath(struct oidc_account*    account,
                 const struct arguments* arguments) {
  return askOrNeedCertPath(account, arguments, 1);
}

void needCertPath(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedCertPath(account, arguments, 0);
}
