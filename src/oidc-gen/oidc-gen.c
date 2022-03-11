#include "oidc-gen.h"

#include <stdlib.h>

#include "gen_handler.h"
#include "utils/accountUtils.h"
#include "utils/commonFeatures.h"
#include "utils/disableTracing.h"
#include "utils/file_io/fileUtils.h"
#include "utils/listUtils.h"
#include "utils/logger.h"

int main(int argc, char** argv) {
  platform_disable_tracing();
  logger_open("oidc-gen");
  logger_setloglevel(NOTICE);

  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.debug) {
    logger_setloglevel(DEBUG);
  }

  assertOidcDirExists();

  if (arguments.listAccounts) {
    common_handleListConfiguredAccountConfigs();
    exit(EXIT_SUCCESS);
  }
  if (arguments.print) {
    gen_handlePrint(arguments.print, &arguments);
    exit(EXIT_SUCCESS);
  }
  if (arguments.updateConfigFile) {
    gen_handleUpdateConfigFile(arguments.updateConfigFile, &arguments);
    exit(EXIT_SUCCESS);
  }
  if (arguments.rename) {
    gen_handleRename(arguments.args[0], &arguments);
    exit(EXIT_SUCCESS);
  }
  if (arguments.codeExchange) {
    handleCodeExchange(&arguments);
    exit(EXIT_SUCCESS);
  }
  common_assertAgent(0);

  if (arguments.state) {
    stateLookUpWithConfigSave(arguments.state, &arguments);
    exit(EXIT_SUCCESS);
  }

  if (arguments.delete) {
    handleDelete(&arguments);
    exit(EXIT_SUCCESS);
  }

  if (arguments.reauthenticate) {
    reauthenticate(arguments.args[0], &arguments);
    exit(EXIT_SUCCESS);
  }

  struct oidc_account* account = NULL;
  if (arguments.only_at) {
    handleOnlyAT(&arguments);
  } else if (arguments.manual) {
    if (arguments.file) {
      account = getAccountFromMaybeEncryptedFile(arguments.file);
    }
    manualGen(account, &arguments);
  } else {
    struct oidc_account* account = registerClient(&arguments);
    if (account) {
      handleGen(account, &arguments, NULL);
    } else {
      secFreeList(arguments.flows);
      exit(EXIT_FAILURE);
    }
  }
  secFreeList(arguments.flows);
  exit(EXIT_SUCCESS);
}
