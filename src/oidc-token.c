#include "oidc-token.h"
#include "../lib/api/oidc-agent-api.h"
#include "privileges/token_privileges.h"
#include "utils/listUtils.h"

int main(int argc, char** argv) {
  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (!arguments.noSeccomp) {
    initOidcTokenPrivileges(&arguments);
  }

  if (arguments.list_accounts) {
    char* accountList = getLoadedAccounts();  // for a list of loaded accounts,
                                              // simply call the api
    if (accountList == NULL) {
      // fprintf(stderr, "Error: %s\n", oidcagent_serror());
      oidcagent_perror();
    } else {
      printf("The following accounts are loaded: %s\n", accountList);
      secFree(accountList);
    }
  }
  if (arguments.args[0]) {
    char* scope_str = listToDelimitedString(arguments.scopes, ' ');
    struct token_response response = getTokenResponse(
        arguments.args[0], arguments.min_valid_period,
        scope_str);  // for getting a valid access token just call the api
    secFree(scope_str);
    if (response.token == NULL) {
      // fprintf(stderr, "Error: %s\n", oidcagent_serror());
      oidcagent_perror();
    } else {
      printf("%s\n", response.token);
      // Use response.issuer to access the issuer_url
    }
    secFreeTokenResponse(response);
  }
  if (arguments.scopes) {
    list_destroy(arguments.scopes);
  }
  return 0;
}
