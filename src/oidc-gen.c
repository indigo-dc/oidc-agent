#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "oidc-gen.h"
#include "gen_handler.h"

int main(int argc, char** argv) {
  openlog("oidc-gen", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));

  struct arguments arguments;
  initArguments(&arguments);
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  assertOidcDirExists();

  if(arguments.codeExchangeRequest) {
    handleCodeExchange(arguments.codeExchangeRequest, arguments.args[0], arguments.verbose);
    exit(EXIT_SUCCESS);
  }

  if(arguments.state) {
    handleStateLookUp(arguments.state);
    exit(EXIT_SUCCESS);
  }

  if(arguments.delete) {
    handleDelete(arguments.args[0]);
    exit(EXIT_SUCCESS);
  } 

  struct oidc_account* account = NULL;
  if(arguments.file) {
    account = accountFromFile(arguments.file); 
  }
  if(arguments.manual) {
    manualGen(account, arguments.args[0], arguments.verbose, arguments.codeFlow);
  } else {
    registerClient(arguments.args[0], arguments.output, arguments.verbose);
  }
  freeAccount(account);
  exit(EXIT_SUCCESS);
}

