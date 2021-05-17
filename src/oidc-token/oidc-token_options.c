#include "oidc-token_options.h"

#include "utils/listUtils.h"
#include "utils/stringUtils.h"

#include <stdlib.h>

#define OPT_SECCOMP 1
#define OPT_NAME 2
#define OPT_AUDIENCE 3
#define OPT_IDTOKEN 4

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"time", 't', "SECONDS", 0,
     "Minimum number of seconds the access token should be valid", 1},
    {"issuer", 'i', "OIDC_ISS", OPTION_ARG_OPTIONAL,
     "Return the issuer associated with the requested access token. If neither "
     "-e nor -o is set and OIDC_ISS is not passed, the issuer is printed to "
     "stdout. Otherwise shell commands are printed that will export the value "
     "into an environment variable. The name of this variable can be set with "
     "OIDC_ISS.",
     1},
    {"expires-at", 'e', "OIDC_EXP", OPTION_ARG_OPTIONAL,
     "Return the expiration time for the requested access token. If neither "
     "-i nor -o is set and OIDC_EXP is not passed, the expiration time is "
     "printed to stdout. Otherwise shell commands are printed that will export "
     "the value into an environment variable. The name of this variable can be "
     "set with OIDC_EXP.",
     1},
    {"token", 'o', "OIDC_AT", OPTION_ARG_OPTIONAL,
     "Return the requested access token. If neither "
     "-i nor -e is set and OIDC_AT is not passed, the token is printed to "
     "stdout (Same behaviour as without this option). Otherwise shell commands "
     "are printed that will export the value "
     "into an environment variable. The name of this variable can be set with "
     "OIDC_AT.",
     1},
    {"env", 'c', 0, 0,
     "This will get all available information (same as -a), but will print "
     "shell commands that export environment variables (default names).  The "
     "result for this option is the same as for using 'oidc-token -oie'. With "
     "the -o -i and -e options the name of each environment variable can be "
     "changed.",
     1},
    {"all", 'a', 0, 0,
     "Return all available information (token, issuer, expiration time). Each "
     "value is printed in one line.",
     1},
    {"force-new", 'f', 0, 0,
     "Forces that a new access token is issued and returned.", 1},

    {0, 0, 0, 0, "Advanced:", 2},
    {"scope", 's', "SCOPE", 0,
     "Scope to be requested for the requested access token. To provide "
     "multiple scopes, use this option multiple times.",
     2},
    {"aud", OPT_AUDIENCE, "AUDIENCE", 0,
     "Audience for the requested access token. Multiple audiences can be "
     "provided as a space separated list",
     2},
#ifndef __APPLE__
    {"seccomp", OPT_SECCOMP, 0, 0,
     "Enables seccomp system call filtering; allowing only predefined system "
     "calls.",
     2},
#endif
    {"name", OPT_NAME, "NAME", 0,
     "This option is intended for other applications / scripts that call "
     "oidc-token to obtain an access token. NAME is the name of this "
     "application and might be displayed to the user.",
     2},
    {"id-token", OPT_IDTOKEN, 0, 0,
     "Returns an id-token instead of an access token. This option is meant as "
     "a "
     "development tool. ID-tokens should not be passed as authorization to "
     "resources.",
     2},

    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    case 's':
      if (arguments->scopes == NULL) {
        arguments->scopes        = list_new();
        arguments->scopes->match = (matchFunction)strequal;
      }
      list_rpush(arguments->scopes, list_node_new(arg));
      break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->min_valid_period = strToInt(arg);
      break;
    case OPT_SECCOMP: arguments->seccomp = 1; break;
    case OPT_IDTOKEN: arguments->idtoken = 1; break;
    case OPT_NAME: arguments->application_name = arg; break;
    case OPT_AUDIENCE: arguments->audience = arg; break;
    case 'i':
      arguments->issuer_env.str   = arg;
      arguments->issuer_env.useIt = 1;
      break;
    case 'o':
      arguments->token_env.str   = arg;
      arguments->token_env.useIt = 1;
      break;
    case 'e':
      arguments->expiration_env.str   = arg;
      arguments->expiration_env.useIt = 1;
      break;
    case 'a': arguments->printAll = 1; break;
    case 'f': arguments->forceNewToken = 1; break;
    case 'c':
      arguments->issuer_env.useIt     = 1;
      arguments->token_env.useIt      = 1;
      arguments->expiration_env.useIt = 1;
      break;
    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 1) {
        argp_usage(state);
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "ACCOUNT_SHORTNAME | ISSUER_URL";

static char doc[] =
    "oidc-token -- A client for oidc-agent for getting OIDC access tokens.";

struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void initArguments(struct arguments* arguments) {
  arguments->min_valid_period     = 0;
  arguments->args[0]              = NULL;
  arguments->scopes               = NULL;
  arguments->application_name     = NULL;
  arguments->audience             = NULL;
  arguments->seccomp              = 0;
  arguments->expiration_env.str   = NULL;
  arguments->expiration_env.useIt = 0;
  arguments->token_env.str        = NULL;
  arguments->token_env.useIt      = 0;
  arguments->issuer_env.str       = NULL;
  arguments->issuer_env.useIt     = 0;
  arguments->printAll             = 0;
  arguments->idtoken              = 0;
  arguments->forceNewToken        = 0;
}
