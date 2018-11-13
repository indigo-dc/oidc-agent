#include "oidc-token-exchange.h"
#include "api/oidc-agent-api.h"
#include "oidc-token-exchange_options.h"
#include "privileges/exchange_privileges.h"

int main(int argc, char** argv) {
  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (!arguments.noSeccomp) {
    initOidcExchangePrivileges(&arguments);
  }
  void* ptr = NULL;
  secFree(ptr);
}
