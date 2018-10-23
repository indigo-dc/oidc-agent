#include "oidc-add.h"

#include "account.h"
#include "add_handler.h"
#include "privileges/add_privileges.h"
#include "utils/fileUtils.h"

#include <syslog.h>

int main(int argc, char** argv) {
  openlog("oidc-add", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));
  struct arguments arguments;

  initArguments(&arguments);

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  initOidcAddPrivileges(&arguments);

  if (arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }
  assertOidcDirExists();
  if (arguments.lock || arguments.unlock) {
    add_handleLock(arguments.lock);
    return EXIT_SUCCESS;
  }
  if (arguments.list) {
    add_handleList();
    return EXIT_SUCCESS;
  }

  char* account = arguments.args[0];
  if (!accountConfigExists(account)) {
    printError("No account configured with that short name\n");
    exit(EXIT_FAILURE);
  }
  if (arguments.print) {
    add_handlePrint(account);
    return EXIT_SUCCESS;
  }

  add_handleAddAndRemove(account, arguments.remove, arguments.lifetime);

  return EXIT_SUCCESS;
}
