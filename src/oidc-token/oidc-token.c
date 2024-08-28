#include "oidc-token.h"

#include "api/api.h"
#include "defines/agent_values.h"
#include "token_handler.h"
#include "utils/disableTracing.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/string/stringUtils.h"

int main(int argc, char** argv) {
  platform_disable_tracing();
  logger_open("oidc-token");
  logger_setloglevel(NOTICE);
  struct arguments arguments;
  initArguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  struct agent_response (*getAgentResponseFnc)(const char*, time_t, const char*,
                                               const char*, const char*) =
      getAgentTokenResponse;
  unsigned char useIssuerInsteadOfShortname = 0;
  if (strstarts(arguments.args[0], "https://")) {
    useIssuerInsteadOfShortname = 1;
  }
  if (arguments.idtoken) {
    token_handleIdToken(useIssuerInsteadOfShortname, arguments.args[0]);
    exit(EXIT_SUCCESS);
  }
  if (arguments.mytoken.useIt) {
    if (useIssuerInsteadOfShortname) {
      printf("Mytokens can only be obtained by account shortname, not by "
             "issuer url.\n");
      exit(EXIT_FAILURE);
    }
    struct agent_response response = getAgentMytokenResponse(
        arguments.args[0], arguments.mytoken.str,
        strValid(arguments.application_name) ? arguments.application_name
                                             : "oidc-token");
    if (response.type == AGENT_RESPONSE_TYPE_ERROR) {
      oidcagent_printErrorResponse(response.error_response);
      secFreeAgentResponse(response);
      exit(EXIT_FAILURE);
    }
    printf("%s\n", response.mytoken_response.token);
    secFreeAgentResponse(response);
    return 0;
  }
  if (useIssuerInsteadOfShortname) {
    getAgentResponseFnc = getAgentTokenResponseForIssuer;
  }
  struct agent_response response = getAgentResponseFnc(
      arguments.args[0],
      arguments.forceNewToken ? FORCE_NEW_TOKEN : arguments.min_valid_period,
      arguments.scopes,
      strValid(arguments.application_name) ? arguments.application_name
                                           : "oidc-token",
      arguments.audience);  // for getting a valid access token just call the
                            // api

  if (response.type == AGENT_RESPONSE_TYPE_ERROR) {
    oidcagent_printErrorResponse(response.error_response);
    //    oidcagent_perror();
    secFreeAgentResponse(response);
    exit(EXIT_FAILURE);
  }
  struct token_response res = response.token_response;
  if (arguments.printAll) {
    printf("%s\n", res.token);
    printf("%s\n", res.issuer);
    printf("%lu\n", res.expires_at);
  } else if ((arguments.expiration_env.useIt + arguments.token_env.useIt +
              arguments.issuer_env.useIt) >
             1) {  // more than one option specified
    printEnvCommands(&arguments, res);
  } else if ((arguments.expiration_env.useIt + arguments.token_env.useIt +
              arguments.issuer_env.useIt) ==
             0) {  // none of these options specified
    if (arguments.bearer) {
      printf("Bearer %s\n", res.token);
    } else if (arguments.auth_header) {
      printf("Authorization: Bearer %s", res.token);
    } else {
      printf("%s\n", res.token);
    }
  } else {  // only one option specified
    if (arguments.issuer_env.useIt) {
      if (arguments.issuer_env.str == NULL) {
        printf("%s\n", res.issuer);
      } else {
        printEnvCommands(&arguments, res);
      }
    }
    if (arguments.token_env.useIt) {
      if (arguments.token_env.str == NULL) {
        printf("%s\n", res.token);
      } else {
        printEnvCommands(&arguments, res);
      }
    }
    if (arguments.expiration_env.useIt) {
      if (arguments.expiration_env.str == NULL) {
        printf("%lu\n", res.expires_at);
      } else {
        printEnvCommands(&arguments, res);
      }
    }
  }
  secFreeTokenResponse(res);
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
    printf("%s=%s; export %s;\n", env_name, response.token, env_name);
  }
  if (arguments->issuer_env.useIt) {
    char* env_name = arguments->issuer_env.str ?: ENV_ISS;
    printf("%s=%s; export %s;\n", env_name, response.issuer, env_name);
  }
  if (arguments->expiration_env.useIt) {
    char* env_name = arguments->expiration_env.str ?: ENV_EXP;
    printf("%s=%ld; export %s;\n", env_name, response.expires_at, env_name);
  }
}
