#include "oidc-add.h"

#include "account/account.h"
#include "add_handler.h"
#include "utils/commonFeatures.h"
#include "utils/config/issuerConfig.h"
#include "utils/disableTracing.h"
#include "utils/file_io/fileUtils.h"
#include "utils/logger.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

int main(int argc, char** argv) {
  platform_disable_tracing();
  logger_open("oidc-add");
  logger_setloglevel(NOTICE);
  struct arguments arguments;

  initArguments(&arguments);

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.debug) {
    logger_setloglevel(DEBUG);
  }

  if (arguments.listConfigured) {
    common_handleListConfiguredAccountConfigs();
    return EXIT_SUCCESS;
  }
  if (arguments.listLoaded) {
    add_handleListLoadedAccounts(&arguments);
    return EXIT_SUCCESS;
  }
  if (arguments.removeAll) {
    add_handleRemoveAll(&arguments);
    return EXIT_SUCCESS;
  }
  common_assertAgent(arguments.remote);
  if (arguments.lock || arguments.unlock) {
    add_handleLock(arguments.lock, &arguments);
    return EXIT_SUCCESS;
  }
  checkOidcDirExists();

  const char*   account                     = arguments.args[0];
  unsigned char useIssuerInsteadOfShortname = 0;
  if (strstarts(account, "https://")) {
    useIssuerInsteadOfShortname = 1;
  }
  if (!useIssuerInsteadOfShortname && !accountConfigExists(account)) {
    if (!(arguments.remove && arguments.remote)) {  // If connected with
                                                    // remote agent a remove
      // uses a shortname that does not exist locally
      oidc_errno = OIDC_ENOACCOUNT;
      oidc_perror();
      exit(EXIT_FAILURE);
    }
  }
  if (arguments.print) {
    if (useIssuerInsteadOfShortname) {
      printError(
          "Cannot use '--%s' with an issuer url instead of a shortname.\n",
          OPT_LONG_PRINT);
      return EXIT_FAILURE;
    }
    add_handlePrint(account, &arguments);
    return EXIT_SUCCESS;
  }

  if (arguments.remove) {
    if (useIssuerInsteadOfShortname) {
      printError(
          "Cannot use '--%s' with an issuer url instead of a shortname.\n",
          OPT_LONG_REMOVE);
      return EXIT_FAILURE;
    }
    add_handleRemove(account, &arguments);
  } else {
    if (useIssuerInsteadOfShortname) {
      const char* issuer = account;
      account            = getDefaultAccountConfigForIssuer(issuer);
      if (account == NULL) {
        printError("Could not determine default account shortname for passed "
                   "issuer url: '%s'\n",
                   issuer);
        return EXIT_FAILURE;
      }
    }
    add_handleAdd(account, &arguments);
  }

  return EXIT_SUCCESS;
}
