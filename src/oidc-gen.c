#include "oidc-gen.h"
#include "gen_handler.h"
#include "add_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

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

  if(arguments.listClients) {
    gen_handleList();
  }
  if(arguments.listAccounts) {
    add_handleList();
  }
  if(arguments.listClients || arguments.listAccounts) {
    exit(EXIT_SUCCESS);
  }

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
  if(arguments.manual) {
    if(arguments.file) {
      account = accountFromFile(arguments.file); 
    }
    manualGen(account, arguments.args[0], arguments.verbose, arguments.flow);
  } else {
    struct oidc_account* account = registerClient(arguments.args[0], arguments.output, arguments.verbose);
    if(account) {
    handleGen(account, arguments.verbose, arguments.flow, NULL);
    }
  }
  freeAccount(account);
  exit(EXIT_SUCCESS);
}

