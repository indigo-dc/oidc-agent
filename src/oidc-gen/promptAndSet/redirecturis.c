#include "promptAndSet.h"

#include "_helper.h"
#include "account/account.h"
#include "utils/listUtils.h"
#include "utils/prompt.h"
#include "utils/uriUtils.h"

void askOrNeedRedirectUris(struct oidc_account*    account,
                           const struct arguments* arguments, int optional) {
  if (readRedirectUris(account, arguments)) {
    return;
  }
  ERROR_IF_NO_PROMPT(optional,
                     ERROR_MESSAGE("redirect uri", OPT_LONG_REDIRECT));
  while (1) {
    list_t* redirect_uris = promptMultiple(
        "Please enter Redirect URIs", "Redirect_uris",
        account_getRedirectUris(account), CLI_PROMPT_NOT_VERBOSE);
    oidc_error_t err = checkRedirectUrisForErrors(redirect_uris);
    if (err == OIDC_SUCCESS) {
      account_setRedirectUris(account, redirect_uris);
      return;
    }
    secFreeList(redirect_uris);
    if (err !=
        OIDC_EERROR) {  // If err == OIDC_EERROR a not valid redirect_uri
                        // was entered, so the user want to provide input.
                        // If err != OIDC_EERROR no input was provided
      if (optional) {
        return;
      }
    }
  }
}

int readRedirectUris(struct oidc_account*    account,
                     const struct arguments* arguments) {
  if (arguments->redirect_uris) {
    account_setRedirectUris(account, arguments->redirect_uris);
    return 1;
  }
  return 0;
}

void askRedirectUris(struct oidc_account*    account,
                     const struct arguments* arguments) {
  return askOrNeedRedirectUris(account, arguments, 1);
}

void needRedirectUris(struct oidc_account*    account,
                      const struct arguments* arguments) {
  return askOrNeedRedirectUris(account, arguments, 0);
}
