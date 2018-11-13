#include "oidc-token-exchange.h"
#include "api/oidc-agent-api.h"
#include "exchange_handler.h"
#include "oidc-token-exchange_options.h"
#include "privileges/exchange_privileges.h"

#include <syslog.h>

int main(int argc, char** argv) {
  openlog("oidc-token-exchange", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));

  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (!arguments.noSeccomp) {
    initOidcExchangePrivileges(&arguments);
  }
  if (arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  if (sizeof(arguments.args) / sizeof(*arguments.args) > 1) {
    handleTokenExchange(&arguments);
  }
}
