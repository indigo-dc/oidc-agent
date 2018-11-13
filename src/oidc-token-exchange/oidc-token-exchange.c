#include "oidc-token-exchange.h"
#include "oidc-token-exchange_options.h"

int main(int argc, char** argv) {
  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (!arguments.noSeccomp) {
    // initOidcTokenPrivileges(&arguments);
  }
}
