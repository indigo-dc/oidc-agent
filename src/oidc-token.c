#include "oidc-token.h"

#include "api.h"
#include "utils/cleaner.h"

int main (int argc, char **argv) {
  struct arguments arguments;
  initArguments(&arguments);
  argp_parse (&argp, argc, argv, 0, 0, &arguments);


  if(arguments.list_accounts) {
    char* accountList = getLoadedAccounts(); // for a list of loaded accounts, simply call the api
    if(accountList==NULL) {
      // fprintf(stderr, "Error: %s\n", oidcagent_serror());
      oidcagent_perror();
    } else {
      printf("The following accounts are loaded: %s\n", accountList);
      clearFreeString(accountList);
    }
  }
  if(arguments.args[0]) {
    char* access_token = getAccessToken(arguments.args[0], arguments.min_valid_period, arguments.scope); // for getting an valid access token just call the api
    if(access_token==NULL) {
      // fprintf(stderr, "Error: %s\n", oidcagent_serror());
      oidcagent_perror();
    } else {
      printf("%s\n", access_token);
      clearFreeString(access_token);
    }
  }
  return 0;
}
