#include "oidc-add.h"
#include "account.h"
#include "add_handler.h"

#include <syslog.h>

int main(int argc, char** argv) {
  openlog("oidc-add", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));
  struct arguments arguments;

  initArguments(&arguments); 

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }
  assertOidcDirExists();

  char* account = arguments.args[0];

  if(!accountConfigExists(account)) {
    printf("No account configured with that short name\n");
    exit(EXIT_FAILURE);
  }
  
  add_handleAddAndRemove(account, arguments.remove);
   
  return EXIT_SUCCESS;
}
