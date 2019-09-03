#include "oidc-token-exchange.h"
#include "exchange_handler.h"
#ifndef __APPLE__
#include "privileges/exchange_privileges.h"
#endif
#include "utils/disableTracing.h"
#include "utils/file_io/fileUtils.h"
#include "utils/logger.h"

int main(int argc, char** argv) {
  platform_disable_tracing();
  logger_open("oidc-token-exchange");
  logger_setloglevel(NOTICE);

  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
#ifndef __APPLE__
  if (arguments.seccomp) {
    initOidcExchangePrivileges(&arguments);
  }
#endif
  if (arguments.debug) {
    logger_setloglevel(DEBUG);
  }
  if (arguments.args[1] != NULL) {  // more than 1 argument provided
    if (arguments.persist) {
      assertOidcDirExists();
    }
    handleTokenExchange(&arguments);
  }
  if (arguments.remove) {
    handleRemove(&arguments);
  } else {
    handleTokenRequest(&arguments);
  }
}
