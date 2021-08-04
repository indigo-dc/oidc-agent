#include "_helper.h"
#include "account/account.h"
#include "promptAndSet.h"
#include "utils/stringUtils.h"

void askOrNeedClientId(struct oidc_account*    account,
                       const struct arguments* arguments, int optional) {
  if (readClientId(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional, ERROR_MESSAGE("client id", OPT_LONG_CLIENTID));
  char* cid =
      _gen_prompt("Client_id", account_getClientId(account), 0, optional);
  if (cid) {
    account_setClientId(account, cid);
  }
}

int readClientId(struct oidc_account*    account,
                 const struct arguments* arguments) {
  if (arguments->client_id) {
    account_setClientId(account, oidc_strcopy(arguments->client_id));
    return 1;
  }
  if (prompt_mode() == 0 && strValid(account_getClientId(account))) {
    return 1;
  }
  return 0;
}

void askClientId(struct oidc_account*    account,
                 const struct arguments* arguments) {
  return askOrNeedClientId(account, arguments, 1);
}

void needClientId(struct oidc_account*    account,
                  const struct arguments* arguments) {
  return askOrNeedClientId(account, arguments, 0);
}
