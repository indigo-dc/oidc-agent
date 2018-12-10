#include "exchange_handler.h"
#include "account/account.h"
#include "api/oidc-agent.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/ipc_values.h"
#include "oidc-token-exchange_options.h"
#include "parse_ipc.h"
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
      printError(
          "Could not auto detect cert path. Please provide one using --cp.");
      secFreeAccount(account);
      exit(EXIT_FAILURE);
    }
  }
  char* config = accountToJSONString(*account);
  char* res =
      ipc_cryptCommunicate(REQUEST_TOKENEXCHANGE, config, arguments->args[4]);
  secFree(config);
  if (arguments->verbose) {
    printNormal("%s\n", res);
  }
  exchange_parseResponse(res, *arguments);
  exit(EXIT_SUCCESS);
}

void handleRemove(struct arguments* arguments) {
  struct oidc_account account = {0};
  account_setName(&account, arguments->args[0], NULL);
  char* account_json = accountToJSONString(account);
  char* res          = ipc_cryptCommunicate(REQUEST_DELETE, account_json);
  secFree(account_json);
  if (arguments->verbose) {
    printNormal("%s\n", res);
  }
  exchange_parseResponse(res, *arguments);
  exit(EXIT_SUCCESS);
}

void handleTokenRequest(struct arguments* arguments) {
  struct token_response response =
      getTokenResponse(arguments->args[0], 60, NULL, "oidc-token-exchange");
  if (response.token == NULL) {
    oidc_perror();
    secFreeTokenResponse(response);
    exit(EXIT_FAILURE);
  }
  printNormal("%s\n", response.token);
  secFreeTokenResponse(response);
  exit(EXIT_SUCCESS);
}
