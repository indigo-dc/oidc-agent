#include "oidc-token.h"

#include "api.h"
#include "defines/agent_values.h"
#include "token_handler.h"
#include "utils/disableTracing.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"
#ifndef __APPLE__
#include "privileges/token_privileges.h"
#endif

int main(int argc, char** argv) {
  platform_disable_tracing();
  logger_open("oidc-token");
  logger_setloglevel(NOTICE);
  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
#ifndef __APPLE__
  if (arguments.seccomp) {
    initOidcTokenPrivileges(&arguments);
  }
#endif

  char* scope_str = listToDelimitedString(arguments.scopes, " ");
  struct token_response (*getTokenResponseFnc)(const char*, time_t, const char*,
                                               const char*, const char*) =
      getTokenResponse3;
  unsigned char useIssuerInsteadOfShortname = 0;
  if (strstarts(arguments.args[0], "https://")) {
    useIssuerInsteadOfShortname = 1;
  }
  if (arguments.idtoken) {
    token_handleIdToken(useIssuerInsteadOfShortname, arguments.args[0]);
    exit(EXIT_SUCCESS);
  }
  if (useIssuerInsteadOfShortname) {
    getTokenResponseFnc = getTokenResponseForIssuer3;
  }
  struct token_response response = getTokenResponseFnc(
      arguments.args[0],
      arguments.forceNewToken ? FORCE_NEW_TOKEN : arguments.min_valid_period,
      scope_str,
      strValid(arguments.application_name) ? arguments.application_name
                                           : "oidc-token",
      arguments.audience);  // for getting a valid access token just call the
                            // api
  secFree(scope_str);

  if (response.token == NULL) {
    // fprintf(stderr, "Error: %s\n", oidcagent_serror());
    oidcagent_perror();
    secFreeTokenResponse(response);
    exit(EXIT_FAILURE);
  }
  if (arguments.printAll) {
    printf("%s\n", response.token);
    printf("%s\n", response.issuer);
    printf("%lu\n", response.expires_at);
  } else if ((arguments.expiration_env.useIt + arguments.token_env.useIt +
              arguments.issuer_env.useIt) >
             1) {  // more than one option specified
    printEnvCommands(&arguments, response);
  } else if ((arguments.expiration_env.useIt + arguments.token_env.useIt +
              arguments.issuer_env.useIt) ==
             0) {  // non of these options sepcified
    printf("%s\n", response.token);
  } else {  // only one option specified
    if (arguments.issuer_env.useIt) {
      if (arguments.issuer_env.str == NULL) {
        printf("%s\n", response.issuer);
      } else {
        printEnvCommands(&arguments, response);
      }
    }
    if (arguments.token_env.useIt) {
      if (arguments.token_env.str == NULL) {
        printf("%s\n", response.token);
      } else {
        printEnvCommands(&arguments, response);
      }
    }
    if (arguments.expiration_env.useIt) {
      if (arguments.expiration_env.str == NULL) {
        printf("%lu\n", response.expires_at);
      } else {
        printEnvCommands(&arguments, response);
      }
    }
  }
  secFreeTokenResponse(response);
  if (arguments.scopes) {
    secFreeList(arguments.scopes);
  }
  return 0;
}

void printEnvCommands(struct arguments*     arguments,
                      struct token_response response) {
  if (arguments == NULL) {
    fprintf(stderr, "passed NULL to %s", __func__);
    return;
  }
  if (arguments->token_env.useIt) {
    char* env_name = arguments->token_env.str ?: ENV_TOKEN;
    fprintf(stdout, "%s=%s; export %s;\n", env_name, response.token, env_name);
  }
  if (arguments->issuer_env.useIt) {
    char* env_name = arguments->issuer_env.str ?: ENV_ISS;
    fprintf(stdout, "%s=%s; export %s;\n", env_name, response.issuer, env_name);
  }
  if (arguments->expiration_env.useIt) {
    char* env_name = arguments->expiration_env.str ?: ENV_EXP;
    fprintf(stdout, "%s=%ld; export %s;\n", env_name, response.expires_at,
            env_name);
  }
}
