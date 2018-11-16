#ifndef OIDC_TOKEN_OPTIONS_H
#define OIDC_TOKEN_OPTIONS_H

#include "list/list.h"
#include "utils/stringUtils.h"

#include <argp.h>
#include <stdlib.h>

#define ENV_TOKEN "OIDC_AT"
#define ENV_ISS "OIDC_ISS"
#define ENV_EXP "OIDC_EXP"

struct optional_arg {
  char* str;
  short useIt;
};

struct arguments {
  char*               args[1]; /* account shortname */
  unsigned long       min_valid_period;
  list_t*             scopes;
  int                 noSeccomp;
  struct optional_arg issuer_env;
  struct optional_arg expiration_env;
  struct optional_arg token_env;
  int                 printAll;
};

#define OPT_NOSECCOMP 1

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"time", 't', "SECONDS", 0,
     "Minimum number of seconds the access token should be valid", 1},
    {"issuer", 'i', "OIDC_ISS", 1,
     "Return the issuer associated with the requested access token. If neither "
     "-e nor -o is set and OIDC_ISS is not passed, the issuer is printed to "
     "stdout. Otherwise shell commands are printed that will export the value "
     "into an environment variable. The name of this variable can be set with "
     "OIDC_ISS.",
     2},
    {"expires-at", 'e', "OIDC_EXP", 1,
     "Return the expiration time for the requested access token. If neither "
     "-i nor -o is set and OIDC_EXP is not passed, the expiration time is "
     "printed to stdout. Otherwise shell commands are printed that will export "
     "the value into an environment variable. The name of this variable can be "
     "set with OIDC_EXP.",
     2},
    {"token", 'o', "OIDC_AT", 1,
     "Return the requested access token. If neither "
     "-i nor -e is set and OIDC_AT is not passed, the token is printed to "
     "stdout (Same behaviour as without this option). Otherwise shell commands "
     "are printed that will export the value "
     "into an environment variable. The name of this variable can be set with "
     "OIDC_AT.",
     2},
    {"env", 'c', 0, 0,
     "This will get all available information (same as -a), but will print "
     "shell commands that export environment variables (default names).  The "
     "result for this option is the same as for using 'oidc-token -oie'. With "
     "the -o -i and -e options the name of each environment variable can be "
     "changed.",
     2},
    {"all", 'a', 0, 0,
     "Return all available information (token, issuer, expiration time). Each "
     "value is printed in one line.",
     2},

    {0, 0, 0, 0, "Advanced:", 2},
    {"scope", 's', "SCOPE", 0,
     "scope to be requested for the requested access token. To provide "
     "multiple scopes, use this option multiple times.",
     2},
    {"no-seccomp", OPT_NOSECCOMP, 0, 0,
     "Disables seccomp system call filtering; allowing all system calls. Use "
     "this option if you get an 'Bad system call' error and hand in a bug "
     "report.",
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
        arguments->scopes->match = (int (*)(void*, void*))strequal;
      }
      list_rpush(arguments->scopes, list_node_new(arg));
      break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->min_valid_period = atoi(arg);
      break;
    case OPT_NOSECCOMP: arguments->noSeccomp = 1; break;
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
    case 'a': arguments->printAll = 1;
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

static char args_doc[] = "ACCOUNT_SHORTNAME";

static char doc[] =
    "oidc-token -- A client for oidc-agent for getting OIDC access tokens.";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static inline void initArguments(struct arguments* arguments) {
  arguments->min_valid_period     = 0;
  arguments->args[0]              = NULL;
  arguments->scopes               = NULL;
  arguments->noSeccomp            = 0;
  arguments->expiration_env.str   = NULL;
  arguments->expiration_env.useIt = 0;
  arguments->token_env.str        = NULL;
  arguments->token_env.useIt      = 0;
  arguments->issuer_env.str       = NULL;
  arguments->issuer_env.useIt     = 0;
  arguments->printAll             = 0;
}

#endif  // OIDC_TOKEN_OPTIONS_H
