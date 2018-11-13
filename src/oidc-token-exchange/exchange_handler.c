#include "exchange_handler.h"
#include "account/account.h"
#include "ipc/communicator.h"
#include "ipc/ipc_values.h"
#include "oidc-token-exchange_options.h"
#include "settings.h"
#include "utils/file_io/file_io.h"

#include <stdlib.h>
#include <syslog.h>

void handleTokenExchange(struct arguments* arguments) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Handle token exchange");
  struct oidc_account* account = secAlloc(sizeof(struct oidc_account));
  account_setName(account, arguments->args[0], NULL);
  account_setIssuerUrl(account, arguments->args[1]);
  account_setClientId(account, arguments->args[2]);
  account_setClientSecret(account, arguments->args[3]);
  account_setAccessToken(account, arguments->args[4]);
  // account_setScope
  if (arguments->cert_path != NULL) {
    account_setCertPath(account, arguments->cert_path);
  } else {
    unsigned int i;
    for (i = 0; i < sizeof(possibleCertFiles) / sizeof(*possibleCertFiles);
         i++) {
      if (fileDoesExist(possibleCertFiles[i])) {
        account_setCertPath(account, oidc_strcopy(possibleCertFiles[i]));
        break;
      }
    }
    if (!strValid(account_getCertPath(*account))) {
      printError("Could not auto detect cert path. Please provide one.");
      secFreeAccount(account);
      exit(EXIT_FAILURE);
    }
  }
  char* config = accountToJSONString(*account);
  char* res    = ipc_communicate(REQUEST_TOKENEXCHANGE, config);
  secFree(config);
  if (arguments->verbose) {
    printNormal(res);
  }
  // parseResponse(res); //TODO
  secFree(res);

  exit(EXIT_SUCCESS);
}
